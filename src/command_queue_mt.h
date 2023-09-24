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

#ifndef ET_COMMAND_QUEUE_MT_H
#define ET_COMMAND_QUEUE_MT_H

// Headers for building as GDExtension plug-in.
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/templates/local_vector.hpp>

using namespace godot;

class CommandQueueMT {
    struct SyncSemaphore {
        Semaphore sem;
        bool in_use = false;
    };

    struct CommandBase {
        virtual void call() = 0;
        virtual void post() {}
        virtual ~CommandBase() {}
    };

    struct SyncCommand : public CommandBase {
        SyncSemaphore *sync_sem = nullptr;

        virtual void post() override {
            sync_sem->sem.post();
        }
    };

    template <class T, class M>
    struct Command0 : public CommandBase {
        T *instance;
        M method;
        virtual void call() override {
            (instance->*method)();
        }
    };

    template <class T, class M, class N>
    struct Command1 : public CommandBase {
        T *instance;
        M method;
        N p_n;
        virtual void call() override {
            (instance->*method)(p_n);
        }
    };

    /* commands that don't return but sync */
    template <class T, class M, class N>
    struct CommandSync : public SyncCommand {
        T *instance;
        M method;
        N p_n;
        virtual void call() override {
            (instance->*method)(p_n);
        }
    };

    enum {
        DEFAULT_COMMAND_MEM_SIZE_KB = 256,
        SYNC_SEMAPHORES = 8
    };

    LocalVector<uint8_t> command_mem;
    SyncSemaphore sync_sems[SYNC_SEMAPHORES];
    Mutex mutex;
    Semaphore *sync = nullptr;

    template <class T>
    T *allocate() {
        // alloc size is size+T+safeguard
        uint32_t alloc_size = ((sizeof(T) + 8 - 1) & ~(8 - 1));
        uint64_t size = command_mem.size();
        command_mem.resize(size + alloc_size + 8);
        *(uint64_t *)&command_mem[size] = alloc_size;
        T *cmd = memnew_placement(&command_mem[size + 8], T);
        return cmd;
    }

    template <class T>
    T *allocate_and_lock() {
        lock();
        T *ret = allocate<T>();
        return ret;
    }

    void _flush() {
        lock();

        uint64_t read_ptr = 0;
        uint64_t limit = command_mem.size();

        while (read_ptr < limit) {
            uint64_t size = *(uint64_t *)&command_mem[read_ptr];
            read_ptr += 8;
            CommandBase *cmd = reinterpret_cast<CommandBase *>(&command_mem[read_ptr]);

            cmd->call(); //execute the function
            cmd->post(); //release in case it needs sync/ret
            cmd->~CommandBase(); //should be done, so erase the command

            read_ptr += size;
        }

        command_mem.clear();
        unlock();
    }

    void lock();
    void unlock();
    void wait_for_flush();
    SyncSemaphore *_alloc_sync_sem();

public:
    /* NORMAL PUSH COMMANDS */
    template <class T, class M>
    void push(T *p_instance, M p_method) {
        Command0<T, M> *cmd = allocate_and_lock<Command0<T, M>>();
        cmd->instance = p_instance;
        cmd->method = p_method;
        unlock();
        if (sync)
            sync->post();
    }

    template <class T, class M, class N>
    void push(T *p_instance, M p_method, N p_n) {
        Command1<T, M, N> *cmd = allocate_and_lock<Command1<T, M, N>>();
        cmd->instance = p_instance;
        cmd->method = p_method;
        cmd->p_n = p_n;
        unlock();
        if (sync)
            sync->post();
    }

    /* PUSH AND RET SYNC COMMANDS*/
    template <class T, class M, class N>
    void push_and_sync(T *p_instance, M p_method, N p_n) {
        SyncSemaphore *ss = _alloc_sync_sem();
        CommandSync<T, M, N> *cmd = allocate_and_lock<CommandSync<T, M, N>>();
        cmd->instance = p_instance;
        cmd->method = p_method;
        cmd->p_n = p_n;
        cmd->sync_sem = ss;
        unlock();
        if (sync)
            sync->post();
        ss->sem.wait();
        ss->in_use = false;
    }

    _FORCE_INLINE_ void flush_if_pending() {
        if (unlikely(command_mem.size() > 0)) {
            _flush();
        }
    }
    void flush_all() {
        _flush();
    }

    void wait_and_flush() {
        ERR_FAIL_COND(!sync);
        sync->wait();
        _flush();
    }

    CommandQueueMT(bool p_sync);
    ~CommandQueueMT();
};

#endif // ET_COMMAND_QUEUE_MT_H
