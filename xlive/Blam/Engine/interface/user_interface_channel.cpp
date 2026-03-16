#include "stdafx.h"
#include "user_interface_channel.h"
#include "user_interface_widget_window.h"

bool c_user_interface_channel::active_or_incoming_screen_exists()
{
    return m_active_screen || m_incoming_screen;
}

void c_user_interface_channel::transition_out()
{
    if (m_active_screen)
    {
        m_active_screen->start_widget_animation(3);
    }
}
