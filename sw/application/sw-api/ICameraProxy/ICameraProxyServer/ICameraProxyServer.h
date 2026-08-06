/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ICAMERAPROXYSERVER_H__
#define __ICAMERAPROXYSERVER_H__

#include <memory>
#include <map>
#include <functional>
#include "ICamera.h"

using tICameraMap = std::map<uint32_t, std::shared_ptr<ICamera>>;
using tICameraTypeMap = std::map<std::string, tICameraMap>;

extern int ICamera_main(int argc, char *argv[]);

#endif // __ICAMERAPROXYSERVER_H__