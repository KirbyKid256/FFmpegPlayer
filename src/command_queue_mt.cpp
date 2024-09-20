/**************************************************************************/
/*  command_queue_mt.cpp                                                  */
/**************************************************************************/
/*                     The original file was part of:                     */
/*                             EIRTeam.FFmpeg                             */
/*                         https://ph.eirteam.moe                         */
/**************************************************************************/
/* Copyright (c) 2023-present Álex Román (EIRTeam) & contributors.        */
/*                                                                        */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/
/* The below code has been modified from the original for purposes of     */
/* code clarity, removing unneeded files and functions, and integrating   */
/* with a different project layout and build tool set, but no significant */
/* changes were made to functionality have been made.                     */
/**************************************************************************/

#include "command_queue_mt.h"

#include <godot_cpp/classes/os.hpp>

void CommandQueueMT::lock() {
    mutex.lock();
}

void CommandQueueMT::unlock() {
    mutex.unlock();
}

void CommandQueueMT::wait_for_flush() {
    // wait one millisecond for a flush to happen
    OS::get_singleton()->delay_usec(1000);
}

CommandQueueMT::SyncSemaphore *CommandQueueMT::_alloc_sync_sem() {
    int idx = -1;

    while (true) {
        lock();
        for (int i = 0; i < SYNC_SEMAPHORES; i++) {
            if (!sync_sems[i].in_use) {
                sync_sems[i].in_use = true;
                idx = i;
                break;
            }
        }
        unlock();

        if (idx == -1) {
            wait_for_flush();
        } else {
            break;
        }
    }

    return &sync_sems[idx];
}

CommandQueueMT::CommandQueueMT() {
    sync = memnew(Semaphore);
}

CommandQueueMT::~CommandQueueMT() {
    if (sync) {
        memdelete(sync);
    }
}
