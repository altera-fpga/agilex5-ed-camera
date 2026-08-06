/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

#include <filesystem>
#include <vector>
#include "ICameraProxyServer.h"


int main (int argc, char** argv)
{
    char fuse_dir[] = "/dev/icamera";
    std::filesystem::create_directory(fuse_dir);
    std::vector<char*> fuse_argv;
    fuse_argv.emplace_back(argv[0]);
    fuse_argv.emplace_back((char*)"-f");
    fuse_argv.emplace_back((char*)"-o");
    fuse_argv.emplace_back((char*)"auto_unmount");
    fuse_argv.emplace_back(fuse_dir);
    return ICamera_main((int)fuse_argv.size(), fuse_argv.data());
}