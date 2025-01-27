// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <app/BufferedReadCallback.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <esp_matter.h>
#include <esp_matter_controller_utils.h>

namespace esp_matter {
namespace controller {

using chip::ScopedNodeId;
using chip::SessionHandle;
using chip::app::AttributePathParams;
using chip::app::BufferedReadCallback;
using chip::app::EventPathParams;
using chip::app::ReadClient;
using chip::Messaging::ExchangeManager;
using esp_matter::client::peer_device_t;

typedef enum {
    READ_ATTRIBUTE = 0,
    READ_EVENT,
} read_command_type_t;

class read_command : public ReadClient::Callback {
public:
    read_command(uint64_t node_id, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_or_event_id,
                 read_command_type_t command_type, attribute_report_cb_t attribute_cb, read_done_cb_t read_cb_done, event_report_cb_t event_cb)
        : m_node_id(node_id)
        , m_command_type(command_type)
        , m_buffered_read_cb(*this)
        , on_device_connected_cb(on_device_connected_fcn, this)
        , on_device_connection_failure_cb(on_device_connection_failure_fcn, this)
        , attribute_data_cb(attribute_cb)
        , read_done_cb(read_cb_done)
        , event_data_cb(event_cb)
    {
        if (command_type == READ_ATTRIBUTE) {
            m_attr_path = AttributePathParams(endpoint_id, cluster_id, attribute_or_event_id);

        } else if (command_type == READ_EVENT) {
            m_event_path = EventPathParams(endpoint_id, cluster_id, attribute_or_event_id);
        }
    }

    ~read_command() {}

    esp_err_t send_command();

    AttributePathParams &get_attr_path() { return m_attr_path; }

    EventPathParams &get_event_path() { return m_event_path; }

    read_command_type_t get_command_type() { return m_command_type; }

    BufferedReadCallback &get_buffered_read_cb() { return m_buffered_read_cb; }

    // ReadClient Callback Interface
    void OnAttributeData(const chip::app::ConcreteDataAttributePath &path, chip::TLV::TLVReader *data,
                         const chip::app::StatusIB &status) override;

    void OnEventData(const chip::app::EventHeader &event_header, chip::TLV::TLVReader *data,
                     const chip::app::StatusIB *status) override;

    void OnError(CHIP_ERROR error) override;

    void OnDeallocatePaths(chip::app::ReadPrepareParams &&aReadPrepareParams) override;

    void OnDone(ReadClient *apReadClient) override;

private:
    uint64_t m_node_id;
    read_command_type_t m_command_type;
    union {
        AttributePathParams m_attr_path;
        EventPathParams m_event_path;
    };
    BufferedReadCallback m_buffered_read_cb;

    static void on_device_connected_fcn(void *context, ExchangeManager &exchangeMgr,
                                        const SessionHandle &sessionHandle);
    static void on_device_connection_failure_fcn(void *context, const ScopedNodeId &peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> on_device_connected_cb;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> on_device_connection_failure_cb;

    attribute_report_cb_t attribute_data_cb;
    read_done_cb_t read_done_cb;
    event_report_cb_t event_data_cb;
    
};

esp_err_t send_read_attr_command(uint64_t node_id, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id);

esp_err_t send_read_event_command(uint64_t node_id, uint16_t endpoint_id, uint32_t cluster_id, uint32_t event_id);

} // namespace controller
} // namespace esp_matter
