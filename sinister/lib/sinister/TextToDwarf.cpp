//===-- TextToDwarf.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements methods to generate DWARF byte code from strings.
//
//===----------------------------------------------------------------------===//

#include "sinister/TextToDwarf.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <cassert>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

enum OperandType {
  uleb128,
  sleb128,
  // s = signed, u = unsigned, int = num bytes:
  s1,
  s2,
  s4,
  s8,
  u1,
  u2,
  u4,
  u8,
  // Target specific address:
  addr,
  // 4 bytes in 32-bit mode, 8 bytes in 64-bit mode:
  word,
  // Not fixed, specified by the preceeding operand:
  variable,
};

using OperandTypeArray = llvm::SmallVector<OperandType, 3>;
static const std::vector<OperandTypeArray> g_OperandTypes = {
    /* 0x00, reserved            */ {},
    /* 0x01, reserved            */ {},
    /* 0x02, reserved            */ {},
    /* 0x03, addr                */ {addr},
    /* 0x04, reserved            */ {},
    /* 0x05, reserved            */ {},
    /* 0x06, deref               */ {},
    /* 0x07, reserved            */ {},
    /* 0x08, const1u             */ {u1},
    /* 0x09, const1s             */ {s1},
    /* 0x0a, const2u             */ {u2},
    /* 0x0b, const2s             */ {u2},
    /* 0x0c, const4u             */ {u4},
    /* 0x0d, const4s             */ {s4},
    /* 0x0e, const8u             */ {u8},
    /* 0x0f, const8s             */ {s8},
    /* 0x10, constu              */ {uleb128},
    /* 0x11, consts              */ {sleb128},
    /* 0x12, dup                 */ {},
    /* 0x13, drop                */ {},
    /* 0x14, over                */ {},
    /* 0x15, pick                */ {u1},
    /* 0x16, swap                */ {},
    /* 0x17, rot                 */ {},
    /* 0x18, xderef              */ {},
    /* 0x19, abs                 */ {},
    /* 0x1a, and                 */ {},
    /* 0x1b, div                 */ {},
    /* 0x1c, minus               */ {},
    /* 0x1d, mod                 */ {},
    /* 0x1e, mul                 */ {},
    /* 0x1f, neg                 */ {},
    /* 0x20, not                 */ {},
    /* 0x21, or                  */ {},
    /* 0x22, plus                */ {},
    /* 0x23, plus_uconst         */ {uleb128},
    /* 0x24, shl                 */ {},
    /* 0x25, shr                 */ {},
    /* 0x26, shra                */ {},
    /* 0x27, xor                 */ {},
    /* 0x28, bra                 */ {s2},
    /* 0x29, eq                  */ {},
    /* 0x2a, ge                  */ {},
    /* 0x2b, gt                  */ {},
    /* 0x2c, le                  */ {},
    /* 0x2d, lt                  */ {},
    /* 0x2e, ne                  */ {},
    /* 0x2f, skip                */ {s2},
    /* 0x30, lit0                */ {},
    /* 0x31, lit1                */ {},
    /* 0x32, lit2                */ {},
    /* 0x33, lit3                */ {},
    /* 0x34, lit4                */ {},
    /* 0x35, lit5                */ {},
    /* 0x36, lit6                */ {},
    /* 0x37, lit7                */ {},
    /* 0x38, lit8                */ {},
    /* 0x39, lit9                */ {},
    /* 0x3a, lit10               */ {},
    /* 0x3b, lit11               */ {},
    /* 0x3c, lit12               */ {},
    /* 0x3d, lit13               */ {},
    /* 0x3e, lit14               */ {},
    /* 0x3f, lit15               */ {},
    /* 0x40, lit16               */ {},
    /* 0x41, lit17               */ {},
    /* 0x42, lit18               */ {},
    /* 0x43, lit19               */ {},
    /* 0x44, lit20               */ {},
    /* 0x45, lit21               */ {},
    /* 0x46, lit22               */ {},
    /* 0x47, lit23               */ {},
    /* 0x48, lit24               */ {},
    /* 0x49, lit25               */ {},
    /* 0x4a, lit26               */ {},
    /* 0x4b, lit27               */ {},
    /* 0x4c, lit28               */ {},
    /* 0x4d, lit29               */ {},
    /* 0x4e, lit30               */ {},
    /* 0x4f, lit31               */ {},
    /* 0x50, reg0                */ {},
    /* 0x51, reg1                */ {},
    /* 0x52, reg2                */ {},
    /* 0x53, reg3                */ {},
    /* 0x54, reg4                */ {},
    /* 0x55, reg5                */ {},
    /* 0x56, reg6                */ {},
    /* 0x57, reg7                */ {},
    /* 0x58, reg8                */ {},
    /* 0x59, reg9                */ {},
    /* 0x5a, reg10               */ {},
    /* 0x5b, reg11               */ {},
    /* 0x5c, reg12               */ {},
    /* 0x5d, reg13               */ {},
    /* 0x5e, reg14               */ {},
    /* 0x5f, reg15               */ {},
    /* 0x60, reg16               */ {},
    /* 0x61, reg17               */ {},
    /* 0x62, reg18               */ {},
    /* 0x63, reg19               */ {},
    /* 0x64, reg20               */ {},
    /* 0x65, reg21               */ {},
    /* 0x66, reg22               */ {},
    /* 0x67, reg23               */ {},
    /* 0x68, reg24               */ {},
    /* 0x69, reg25               */ {},
    /* 0x6a, reg26               */ {},
    /* 0x6b, reg27               */ {},
    /* 0x6c, reg28               */ {},
    /* 0x6d, reg29               */ {},
    /* 0x6e, reg30               */ {},
    /* 0x6f, reg31               */ {},
    /* 0x70, breg0               */ {sleb128},
    /* 0x71, breg1               */ {sleb128},
    /* 0x72, breg2               */ {sleb128},
    /* 0x73, breg3               */ {sleb128},
    /* 0x74, breg4               */ {sleb128},
    /* 0x75, breg5               */ {sleb128},
    /* 0x76, breg6               */ {sleb128},
    /* 0x77, breg7               */ {sleb128},
    /* 0x78, breg8               */ {sleb128},
    /* 0x79, breg9               */ {sleb128},
    /* 0x7a, breg10              */ {sleb128},
    /* 0x7b, breg11              */ {sleb128},
    /* 0x7c, breg12              */ {sleb128},
    /* 0x7d, breg13              */ {sleb128},
    /* 0x7e, breg14              */ {sleb128},
    /* 0x7f, breg15              */ {sleb128},
    /* 0x80, breg16              */ {sleb128},
    /* 0x81, breg17              */ {sleb128},
    /* 0x82, breg18              */ {sleb128},
    /* 0x83, breg19              */ {sleb128},
    /* 0x84, breg20              */ {sleb128},
    /* 0x85, breg21              */ {sleb128},
    /* 0x86, breg22              */ {sleb128},
    /* 0x87, breg23              */ {sleb128},
    /* 0x88, breg24              */ {sleb128},
    /* 0x89, breg25              */ {sleb128},
    /* 0x8a, breg26              */ {sleb128},
    /* 0x8b, breg27              */ {sleb128},
    /* 0x8c, breg28              */ {sleb128},
    /* 0x8d, breg29              */ {sleb128},
    /* 0x8e, breg30              */ {sleb128},
    /* 0x8f, breg31              */ {sleb128},
    /* 0x90, regx                */ {uleb128},
    /* 0x91, fbreg               */ {sleb128},
    /* 0x92, bregx               */ {uleb128, sleb128},
    /* 0x93, piece               */ {uleb128},
    /* 0x94, deref_size          */ {u1},
    /* 0x95, xderef_size         */ {u1},
    /* 0x96, nop                 */ {},
    /* 0x97, push_object_address */ {},
    /* 0x98, call2               */ {u2},
    /* 0x99, call4               */ {u4},
    /* 0x9a, call_ref            */ {word},
    /* 0x9b, form_tls_address    */ {},
    /* 0x9c, call_frame_cfa      */ {},
    /* 0x9d, bit_piece           */ {uleb128, uleb128},
    /* 0x9e, implicit_value      */ {uleb128, variable},
    /* 0x9f, stack_value         */ {},
    /* 0xa0, implicit_pointer    */ {word, sleb128},
    /* 0xa1, addrx               */ {uleb128},
    /* 0xa2, constx              */ {uleb128},
    /* 0xa3, entry_value         */ {uleb128, variable},
    /* 0xa4, const_type          */ {uleb128, u1, variable},
    /* 0xa5, regval_type         */ {uleb128, uleb128},
    /* 0xa6, deref_type          */ {u1, uleb128},
    /* 0xa7, xderef_type         */ {u1, uleb128},
    /* 0xa8, convert             */ {uleb128},
    /* 0xa9, reinterpret         */ {uleb128},
    /* 0xe0, DW_OP_lo_user       */ {},
    /* 0xff, DW_OP_lo_user       */ {},
};

OperandTypeArray const &getOperandTypes(uint8_t Opcode) {
  assert(Opcode < g_OperandTypes.size() && "Expected valid opcode");
  return g_OperandTypes[Opcode];
}

// Handily, LLVM already gives us all the DWARF operator names and codes
// in llvm/BinaryFormat/Dwarf.def
// HANDLE_DW_OP(ID, NAME, VERSION, VENDOR)
#define HANDLE_DW_OP(OPCODE, NAME, UNUSED0, UNUSED1)                           \
  {std::string("DW_OP_" #NAME), OPCODE},
std::unordered_map<std::string, uint8_t> g_StrToOpcode = {
#include "llvm/BinaryFormat/Dwarf.def"
};
#undef HANDLE_DW_OP
#define HANDLE_DW_OP(OPCODE, NAME, UNUSED0, UNUSED1)                           \
  {OPCODE, std::string("DW_OP_" #NAME)},
std::unordered_map<uint8_t, std::string> g_OpcodeToStr = {
#include "llvm/BinaryFormat/Dwarf.def"
};
#undef HANDLE_DW_OP

static std::optional<uint8_t> getOpcode(std::string_view Name) {
  auto R = g_StrToOpcode.find(std::string(Name));
  if (R != g_StrToOpcode.end())
    return R->second;
  return std::nullopt;
}

struct SrcLoc {
  unsigned Line = 1;
  unsigned Column = 1;
};

struct Token {
  enum Type { Opcode, Int, HexInt, Comma, Error, LParen, RParen, } Ty;
  std::string_view lexeme;
  SrcLoc Loc;
  uint8_t Code;
  Token(Type Ty, std::string_view lexeme, SrcLoc Loc, uint8_t Code = 0)
      : Ty(Ty), lexeme(lexeme), Loc(Loc), Code(Code) {}

  uint8_t radix() const {
    assert(Ty == Int || Ty == HexInt);
    return Ty == Token::Int ? 10 : 16;
  }
};

static void printError(SrcLoc Loc, std::string_view Msg) {
  std::cerr << "ERROR at line " << Loc.Line << ", col " << Loc.Column << ": "
            << Msg << "\n";
}

class Lexer {
  std::string_view Text;
  unsigned Start = 0;
  unsigned Next = 0;
  SrcLoc StartLoc;
  SrcLoc NextLoc;

  bool atEnd() { return Next >= Text.size(); }
  char peek() { return Text[Next]; }
  char advance() {
    char Ch = peek();
    Next += 1;
    NextLoc.Column += 1;
    return Ch;
  }
  bool match(char Ch) {
    if (atEnd() || peek() != Ch)
      return false;
    advance();
    return true;
  }
  bool matchEndline() {
    // Sequences: \r\n, \n, \r, \v
    bool CR = match('\r');
    return match('\n') || CR || match('\v');
  }
  static Token error(SrcLoc Loc, std::string_view Msg) {
    printError(Loc, Msg);
    return Token(Token::Error, std::string_view(), Loc);
  }
  static Token error(SrcLoc Loc, std::string_view Expected, char Got) {
    std::stringstream Err;
    Err << "ERROR at line " << Loc.Line << ", col " << Loc.Column
        << ": Expected " << Expected << " but got " << Got << "\n";
    printError(Loc, Err.str());
    return Token(Token::Error, std::string_view(), Loc);
  }
  void eatWhitespaceAndComments() {
    while (!atEnd()) {
      if (matchEndline()) {
        NextLoc.Column = 1;
        NextLoc.Line += 1;
        continue;
      } else if (match(' ') || match('\t')) {
        continue;
      } else if (match('#')) {
        // Comments start with # and continue to the end of the line.
        while (!matchEndline())
          advance();
        continue;
      }
      // Not whitespace - end.
      break;
    }
  }
  std::string_view getCurrentSubstr() {
    return Text.substr(Start, Next - Start);
  }

  Token create(Token::Type Type) {
    return Token(Type, getCurrentSubstr(), StartLoc);
  }
  Token createOpcode(uint8_t Code) {
    return Token(Token::Opcode, getCurrentSubstr(), StartLoc, Code);
  }

  Token finishNumber(char First) {
    bool Negative = First == '-';
    if (Negative && match('0') && match('x'))
      return error(StartLoc, "Negative hexidecimal literals unsupported");
    bool Hex = First == '0' && match('x');
    while (!atEnd()) {
      char Ch = peek();
      if (isspace(Ch))
        break;
      if ((!Hex && !std::isdigit(Ch)) || (Hex && !std::isxdigit(Ch)))
        break;
      advance();
    }
    if (Hex) {
      // Chop off the 0x.
      Start += 2;
      return create(Token::HexInt);
    } else {
      return create(Token::Int);
    }
  }

  Token finishOpcode() {
    // Advance to next space.
    while (!atEnd()) {
      char Ch = peek();
      if (isspace(Ch))
        break;
      if (!std::isalnum(Ch) && Ch != '_')
        break;
      advance();
    }
    std::string_view Str = getCurrentSubstr();
    if (auto Code = getOpcode(Str))
      return createOpcode(*Code);
    return error(StartLoc,
                 std::string("Unknown operation: '") + std::string(Str) + "'");
  }

  Token getNext() {
    char Ch = advance();
    if (Ch == ',')
      return create(Token::Comma);
    if (Ch == '(')
      return create(Token::LParen);
    if (Ch == ')')
      return create(Token::RParen);
    if (Ch == '-' || Ch >= '0' && Ch <= '9')
      return finishNumber(Ch);
    // Otherwise, expect a DWARF opcode.
    if (Ch == 'D')
      return finishOpcode();
    return error(StartLoc, std::string("Unexpected character: ") + Ch);
  }

public:
  Lexer(std::string_view Text) : Text(Text) {
    StartLoc.Line = 1;
    StartLoc.Column = 1;
  }

  std::optional<llvm::SmallVector<Token>> lex() {
    llvm::SmallVector<Token> Output;
    while (!atEnd()) {
      eatWhitespaceAndComments();
      Start = Next;
      StartLoc = NextLoc;
      if (atEnd())
        break;
      Token Tok = getNext();
      if (Tok.Ty == Token::Error)
        return std::nullopt;
      Output.push_back(Tok);
    }
    return Output;
  }
};

static llvm::StringRef toRef(std::string_view Str) {
  return llvm::StringRef(Str.data(), Str.size());
}

static void encodeAPInt(llvm::APInt const &Int,
                        std::vector<uint8_t> *Target) {
  char const *RawData = (char const *)Int.getRawData();
  int NumBytes = (Int.getBitWidth() + 7) / 8;
  for (int i = 0; i < NumBytes; ++i)
    Target->push_back(RawData[i]);
}

// FIXME: Reduce code by doing lex + parse + encode in one pass.
struct Parser {
  llvm::SmallVector<Token> const &Toks;
  std::vector<uint8_t> Result;

  llvm::APInt PreviousOperandValue;
  /// Save operand value so that variable-length encoded operands that have a
  /// length specified by this operand can read it.
  void saveOperandValue(llvm::APInt const &Value) {
    PreviousOperandValue = Value;
  }
  llvm::APInt const &getPreviousOperandValue() {
    return PreviousOperandValue;
  }

  unsigned Next = 0;
  static bool error(SrcLoc Loc, std::string_view Msg) {
    std::cerr << "ERROR at line " << Loc.Line << ", col " << Loc.Column << ": "
              << Msg << "\n";
    return false;
  }
  bool atEnd() { return Next >= Toks.size(); }
  Token const &peek() { return Toks[Next]; }
  Token const &previous() { return Toks[Next - 1]; }
  Token const &advance() {
    Token const &Tok = peek();
    ++Next;
    return Tok;
  }
  bool consume(Token::Type Ty, std::string const &Err) {
    Token const &Tok = advance();
    if (Tok.Ty == Ty)
      return true;
    error(Tok.Loc, Err);
    return false;
  }
  bool match(Token::Type Ty) {
    if (peek().Ty != Ty)
      return false;
    advance();
    return true;
  }

  static bool isNegative(Token Operand) { return Operand.lexeme[0] == '-'; }

  static unsigned getBitsNeededForOperand(Token Op, bool Signed) {
    unsigned BitsNeeded =
        llvm::APInt::getBitsNeeded(toRef(Op.lexeme), Op.radix());
    // getBitsNeeded counts strings without '-' as unsigned. Hex is parsed
    // as unsigned and bitcast to signed.
    if (Signed && isNegative(Op) && Op.Ty != Token::HexInt)
      BitsNeeded += 1;
    return BitsNeeded;
  }
  static bool operandValueFits(unsigned TypeBitWidth, Token Operand,
                               bool Signed) {
    unsigned BitsNeeded = getBitsNeededForOperand(Operand, Signed);
    // TODO: Make the error functions less awkward to use.
    if (BitsNeeded > TypeBitWidth)
      return error(Operand.Loc, "Operand value too large. It requires " +
                                    std::to_string(BitsNeeded) +
                                    " bits but the operand type is " +
                                    std::to_string(TypeBitWidth) +
                                    " bits wide.");
    return true;
  }

  bool encodeInt(Token Operand, unsigned TypeBitWidth, bool Signed) {
    if (!Signed && isNegative(Operand))
      return error(Operand.Loc, "Expected unsigned operand");

    if (!operandValueFits(TypeBitWidth, Operand, Signed))
      return false;
    // We've already checked it will fit.
    llvm::APInt Value(TypeBitWidth, toRef(Operand.lexeme), Operand.radix());
    saveOperandValue(Value);
    encodeAPInt(Value, &Result);
    return true;
  }

  bool encodeLEB128(Token Operand, bool Signed) {
    if (!Signed && isNegative(Operand))
      return error(Operand.Loc, "Expected unsigned operand");

    unsigned BitsNeeded = getBitsNeededForOperand(Operand, Signed);
    llvm::APInt Value(BitsNeeded, toRef(Operand.lexeme), Operand.radix());
    saveOperandValue(Value);
    // zext size to a multiple of 7. Do this after saving the operand value
    // because it's just an implementation detail for the encoding algorithm.
    Value = Value.zext(BitsNeeded + 7 - (BitsNeeded % 7));

    // Next group of 7 bytes start position.
    unsigned GroupStart = 0;
    do {
      uint8_t Byte = Value.extractBitsAsZExtValue(7, GroupStart);
      GroupStart += 7;
      // if not finished, set high bit of byte.
      if (GroupStart != Value.getBitWidth())
        Byte |= 0b10000000;
      Result.push_back(Byte);
    } while (GroupStart != Value.getBitWidth());
    return true;
  }

  bool encodeVariableLengthOperand(Token Operand) {
    llvm::APInt const &Previous = getPreviousOperandValue();
    if (Previous.getBitWidth() > 64)
      return error(Operand.Loc, "The size operand for this variable-length "
                                "operand (the preceeding operand) is larger "
                                "than 64 bits, which isn't supported yet");

    // Previous operand encodes the number of bytes of this operand.
    uint64_t BitWidth = Previous.getZExtValue() * 8;

    if (isNegative(Operand))
      return error(Operand.Loc, "Expected unsigned operand");
    if (!operandValueFits(BitWidth, Operand, /*Signed*/false))
      return false; // Error already printed.

    llvm::APInt Value(BitWidth, toRef(Operand.lexeme), Operand.radix());
    // We don't really need to do this, as a variable length int is not ever
    // used to specify the length of another. Do it anyway, as it's one less
    // gotcha if we do something weird later.
    saveOperandValue(Value);
    encodeAPInt(Value, &Result);
    return true;
  }

  bool encodeOperand(OperandType Ty, Token Operand) {
    switch (Ty) {
    case s1:
      return encodeInt(Operand, 8, true);
    case s2:
      return encodeInt(Operand, 16, true);
    case s4:
      return encodeInt(Operand, 32, true);
    case s8:
      return encodeInt(Operand, 64, true);
    case u1:
      return encodeInt(Operand, 8, false);
    case u2:
      return encodeInt(Operand, 16, false);
    case u4:
      return encodeInt(Operand, 32, false);
    case u8:
      return encodeInt(Operand, 64, false);
    case addr:
      // FIXME: Determine / specify bit width of target address.
      // Assume 64-bit for now.
      return encodeInt(Operand, 64, false);
    case word:
      // FIXME: Determine / specify DWARF bit-mode. Assume 32-bit mode
      // for now.
      return encodeInt(Operand, 32, false);
    case uleb128:
      return encodeLEB128(Operand, false);
    case sleb128:
      return encodeLEB128(Operand, true);
    case variable:
      // FIXME: This doesn't help much with DW_OP_entry_value, which essentially
      // expects another DWARF expression as its operand, which is then encoded
      // with the operand pair. DW_OP_const_type also needs a DIE reference
      // as the first operand, which will require some extra scaffolding.
      return encodeVariableLengthOperand(Operand);
    };
    return false;
  }

  bool encodeOperation() {
    // Already eaten the first tok.
    Token const &OpTok = previous();
    assert(OpTok.Ty == Token::Opcode);

    // Add the opcode.
    Result.push_back(OpTok.Code);

    auto const &Operands = getOperandTypes(OpTok.Code);

    // No parens for zero-operand opcodes.
    if (Operands.empty()) {
      // Give helpful error if there's a '('.
      if (!atEnd() && peek().Ty == Token::LParen) {
        error(peek().Loc, "Don't use parens after zero-operand opcodes");
        return false;
      }
      return true;
    }

    if (!consume(Token::LParen, "Expected '(' after opcode with operands"))
      return false;

    // Parse and encode the operands.
    // Pos: one-based operand index.
    int Pos = 0;
    for (OperandType OperandTy : Operands) {
      ++Pos;
      if (!match(Token::HexInt) && !consume(Token::Int, "Expected int param"))
        return false;

      if (!encodeOperand(OperandTy, previous()))
        return false;

      // TODO: Could give useful hint about num operands.
      if (Pos != Operands.size())
        if (!consume(Token::Comma, "Expected comma after operand"))
          return false;
    }

    if (!consume(Token::RParen, "Expected ')' after operand list"))
      return false;

    return /*Success*/ true;
  }

public:
  Parser(llvm::SmallVector<Token> const &Toks) : Toks(Toks) {}
  std::optional<std::vector<uint8_t>> parse() {
    while (!atEnd()) {
      if (!consume(Token::Opcode, "Expected an opcode"))
        return std::nullopt;
      if (!encodeOperation())
        return std::nullopt;
    }
    return Result;
  }
};

std::optional<std::vector<uint8_t>>
parseDwarfExpression(std::string const &Input) {
  Lexer L(Input);
  if (auto OptionalToks = L.lex()) {
    Parser P(*OptionalToks);
    if (auto OptionalBytes = P.parse())
      return *OptionalBytes;
  }
  return std::nullopt;
}