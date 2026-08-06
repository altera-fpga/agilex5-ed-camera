/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AtUtils.h"
#include "SelectEvent.h"
#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#endif

SelectEvent::SelectEvent()
:	_valid(false)
{
#ifdef __linux__
    int r = ::pipe(_fds);
    (void)r;
    r = fcntl(_fds[1], F_SETFL, O_NONBLOCK);
    _valid = (r == 0);
#else
    _fds[0] = 100;
    _fds[1] = 101;
    _valid = true;
#endif
}

SelectEvent::~SelectEvent()
{
#ifdef __linux__
    if (_valid)
    {
        AtUtils::o_close(_fds[0]);
        AtUtils::o_close(_fds[1]);
        _fds[0] = _fds[1] = 0;
        _valid = false;
    }
#else
    _fds[0] = _fds[1] = 0;
    _valid = false;
#endif
}

SelectEvent::operator int()
{
    return _fds[0];
}

void SelectEvent::Set()
{
#ifdef __linux__
    if (_valid)
    {
        int r = ::write(_fds[1], "1", 1);
        (void)r;
    }
#else
    int fds_0 = _fds[0];
    int fds_1 = _fds[1];
    int text = 0;
#endif
}

