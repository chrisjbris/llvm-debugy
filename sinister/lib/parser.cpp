#include <unordered_map>
#include <string>
#include <string_view>
#include <cassert>
#include <optional>
#include <vector>
#include <cctype>
#include <iomanip>
#include <iostream>

std::optional<uint8_t> getOpcode(std::string_view Name) {
    // Handily, LLVM already gives us all the DWARF operator names and codes
    // in llvm/BinaryFormat/Dwarf.def
    // HANDLE_DW_OP(ID, NAME, VERSION, VENDOR)
    #define HANDLE_DW_OP(OPCODE, NAME, UNUSED0, UNUSED1) \
        { std::string("DW_OP_" #NAME), OPCODE},
    std::unordered_map<std::string, uint8_t> Opcodes = {
        #include "llvm/BinaryFormat/Dwarf.def"
    };
    #undef HANDLE_DW_OP
    auto R = Opcodes.find(std::string(Name));
    if (R != Opcodes.end())
        return R->second;
    return std::nullopt;
}

std::string_view eatNextWord(std::string_view &Remaining) {
    unsigned Start = 0;
    while (Start < Remaining.size()) {
        char Ch = Remaining[Start];
        // Just ignore commas for now.
        if (!isspace(Ch) && Ch != ',')
            break;
        ++Start;
    }
    unsigned End = Start;
    while (End < Remaining.size()) {
        char Ch = Remaining[End];
        if (isspace(Ch) || Ch == ',')
            break;
        ++End;
    }

    auto Result = Remaining.substr(Start, End - Start);
    Remaining = Remaining.substr(End, Remaining.size() - End);
    return Result;
}

// Dumb bare-bones string -> opcodes.
// No useful errors, no non-opcode lexemes (e.g. uleb int operands).
std::optional<std::vector<uint8_t>> parseExpression(
                                        std::string_view ExpressionString) {
    std::vector<uint8_t> Arr;
    while (!ExpressionString.empty()) {
        auto Word = eatNextWord(ExpressionString);
        if (auto MaybeOpCode = getOpcode(Word)) {
            Arr.push_back(*MaybeOpCode);
        } else {
            std::cerr << "Err: " << Word << " unknown opcode\n";
            return std::nullopt; // Provide useful errors.
        }
    }
    return Arr;
}

void test() {
    std::string Expr = "DW_OP_breg1 DW_OP_lit0 DW_OP_plus DW_OP_stack_value";
    if (auto Arr = parseExpression(Expr)) {
        for (auto E : *Arr)
            std::cout << std::hex << (uint32_t)E << " ";
        std::cout << "\n:)\n";
    } else {
        std::cout << ":(\n";
    }
}