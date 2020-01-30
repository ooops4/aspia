//
// Aspia Project
// Copyright (C) 2020 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#ifndef IPC__SHARED_MEMORY_FACTORY_H
#define IPC__SHARED_MEMORY_FACTORY_H

#include "base/macros_magic.h"

#include <memory>

namespace ipc {

class SharedMemory;

class SharedMemoryFactory
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        // Called when a shared memory is successfully created or opened.
        virtual void onSharedMemoryCreate(int id) = 0;

        // Called when the shared memory is destroyed.
        virtual void onSharedMemoryDestroy(int id) = 0;
    };

    explicit SharedMemoryFactory(Delegate* delegate);
    ~SharedMemoryFactory();

    // Creates a new shared memory. If an error occurs, nullptr is returned.
    std::unique_ptr<SharedMemory> create(size_t size);

    // Opens an existing shared memory.
    // If shared memory does not exist, nullptr is returned.
    // If any other error occurs, nullptr is returned.
    std::unique_ptr<SharedMemory> open(int id);

private:
    friend class SharedMemoryFactoryProxy;
    void onSharedMemoryCreate(int id);
    void onSharedMemoryDestroy(int id);

    std::shared_ptr<SharedMemoryFactoryProxy> factory_proxy_;
    Delegate* delegate_;

    DISALLOW_COPY_AND_ASSIGN(SharedMemoryFactory);
};

} // namespace ipc

#endif // IPC__SHARED_MEMORY_FACTORY_H