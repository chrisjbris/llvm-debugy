//===-- FileReader.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines functions to read files.
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sinister/utils/FileReader.h"

bool FileToString(std::string FileName, std::string &Dest) {
    struct stat sb{};

    int fd = open(FileName.c_str(), O_RDONLY);
    if (fd < 0)
        return false;

    fstat(fd, &sb);
    Dest.resize(sb.st_size);
    read(fd, (char*)(Dest.data()), sb.st_size);
    close(fd);
    return true;
}