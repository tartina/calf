/* Calf DSP Library
 * LV2-related helper classes and functions
 *
 * Copyright (C) 2001-2008 Krzysztof Foltman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef CALF_LV2HELPERS_H
#define CALF_LV2HELPERS_H

#if USE_LV2

/// A mixin for adding the event feature, URI map and MIDI event type retrieval to small plugins
/// @todo refactor into separate event feature, URI map and MIDI event type mixins some day
template<class T>
class midi_mixin: public T
{
public:
    LV2_URI_Map_Feature *uri_map;
    uint32_t midi_event_type;
    LV2_Event_Feature *event_feature;
    virtual void use_feature(const char *URI, void *data) {
        if (!strcmp(URI, LV2_URI_MAP_URI))
        {
            uri_map = (LV2_URI_Map_Feature *)data;
            midi_event_type = uri_map->uri_to_id(uri_map->callback_data, 
                "http://lv2plug.in/ns/ext/event",
                "http://lv2plug.in/ns/ext/midi#MidiEvent");
        }
        else if (!strcmp(URI, LV2_EVENT_URI))
        {
            event_feature = (LV2_Event_Feature *)data;
        }
        T::use_feature(URI, data);
    }
};

/// LV2 event structure + payload as 0-length array for easy access
struct lv2_event: public LV2_Event
{
    uint8_t data[0];
    inline lv2_event &operator=(const lv2_event &src) {
        *(LV2_Event *)this = (const LV2_Event &)src;
        memcpy(data, src.data, src.size);
        return *this;
    }
};

/// A read-only iterator-like object for reading from event buffers
class event_port_read_iterator
{
protected:
    const LV2_Event_Buffer *buffer;
    uint32_t offset;
public:
    /// Default constructor creating a useless iterator you can assign to
    event_port_read_iterator()
    : buffer(NULL)
    , offset(0)
    {
    }
    
    /// Create an iterator based on specified buffer and index/offset values
    event_port_read_iterator(const LV2_Event_Buffer *_buffer, uint32_t _offset = 0)
    : buffer(_buffer)
    , offset(0)
    {
    }

    /// Are any data left to be read?
    inline operator bool() const {
        return offset < buffer->size;
    }
    
    /// Read pointer
    inline const lv2_event &operator*() const {
        return *(const lv2_event *)(buffer->data + offset);
    }

    /// Move to the next element
    inline event_port_read_iterator operator++() {
        offset += ((**this).size + 19) &~7;
        return *this;
    }

    /// Move to the next element
    inline event_port_read_iterator operator++(int) {
        event_port_read_iterator old = *this;
        offset += ((**this).size + 19) &~7;
        return old;
    }
};

/// A write-only iterator-like object for writing to event buffers
class event_port_write_iterator
{
protected:
    LV2_Event_Buffer *buffer;
public:
    /// Default constructor creating a useless iterator you can assign to
    event_port_write_iterator()
    : buffer(NULL)
    {
    }
    
    /// Create a write iterator based on specified buffer and index/offset values
    event_port_write_iterator(LV2_Event_Buffer *_buffer)
    : buffer(_buffer)
    {
    }

    /// @return the remaining buffer space
    inline uint32_t space_left() const {
        return buffer->capacity - buffer->size;
    }
    /// @return write pointer
    inline lv2_event &operator*() {
        return *(lv2_event *)(buffer->data + buffer->size);
    }
    /// Move to the next element after the current one has been written (must be called after each write)
    inline event_port_write_iterator operator++() {
        buffer->size += ((**this).size + 19) &~7;
        buffer->event_count ++;
        return *this;
    }
    /// Move to the next element after the current one has been written
    inline lv2_event *operator++(int) {
        lv2_event *ptr = &**this;
        buffer->size += ((**this).size + 19) &~7;
        buffer->event_count ++;
        return ptr;
    }
};

#endif
#endif