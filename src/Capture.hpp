
#pragma once

#include <glib-object.h>
#include <gdk/gdk.h>

#pragma once

class Capture
{
public:
    Capture() = default;
    explicit Capture(const Capture& orig) = delete;
    virtual ~Capture() = default;

    virtual GdkPixbuf *get_pixbuf(GdkRectangle* rectangle) = 0;

    bool get_take_window_shot() {
        return m_take_window_shot;
    }
    void set_take_window_shot(bool take_window_shot) {
        m_take_window_shot = take_window_shot;
    }
    bool get_take_area_shot() {
        return m_take_area_shot;
    }
    void set_take_area_shot(bool take_area_shot) {
        m_take_area_shot = take_area_shot;
    }
    bool get_include_pointer() {
        return m_include_pointer;
    }
    void set_include_pointer(bool include_pointer) {
        m_include_pointer = include_pointer;
    }
private:
protected:
    bool m_take_window_shot{false};
    bool m_take_area_shot{false};
    bool m_include_pointer{false};

};

