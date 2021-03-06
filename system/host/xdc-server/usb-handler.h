// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <set>
#include <map>
#include <vector>

namespace xdc {

class UsbHandler {
    // This is required by the UsbHandler constructor, to stop clients calling it directly.
    struct ConstructorTag { explicit ConstructorTag() = default; };

public:
    class Transfer {
    public:
        // Create should be called instead. This is public for make_shared.
        explicit Transfer(ConstructorTag tag) {}

        unsigned char* data() const { return data_; }
        int actual_length() const { return actual_length_; }

    private:
        // Only UsbHandler should create transfers.
        static std::unique_ptr<Transfer> Create();

        // TODO(jocelyndang): this should store a libusb_transfer instead.
        unsigned char* data_;
        int actual_length_;

        friend class UsbHandler;
    };

    // Create should be called instead. This is public for make_unique.
    explicit UsbHandler(ConstructorTag tag) {}

    static std::unique_ptr<UsbHandler> Create();

    // Handles any pending events.
    //
    // Parameters:
    // completed_reads  A vector which will be populated with the usb transfers containing data
    //                  read from the xdc device. Once the client has finished processing a read,
    //                  it should be returned back to the UsbHandler by calling RequeueRead.
    //
    // Returns whether the usb handler fds have changed.
    // If true, the newly added or removed fds should be fetched via GetFdUpdates.
    bool HandleEvents(std::vector<std::unique_ptr<Transfer>>& completed_reads);

    // Returns the read transfer back to the UsbHandler to be requeued.
    void RequeueRead(std::unique_ptr<Transfer> transfer);

    // Populates added_fds and removed_fds with the fds that have been added
    // and removed since GetFdUpdates was last called.
    //
    // Parameters:
    // added_fds      A map that will be populated with fds to start monitoring and
    //                the corresponding events to monitor for.
    // removed_fds    A set that will be populated with fds to stop monitoring.
    //                The fds will be disjoint from added_fds.
    void GetFdUpdates(std::map<int, short>& added_fds, std::set<int>& removed_fds);

    // Returns whether the given file descriptor is currently valid for the usb handler.
    bool IsValidFd(int fd) const { return fds_.count(fd); }

private:
    // All the libusb fds.
    std::set<int> fds_;
};

}  // namespace xdc
