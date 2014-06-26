/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "mir/frontend/session_mediator_report.h"
#include "mir/display_server.h"
#include "mir/run_mir.h"

#include "mir_test_framework/in_process_server.h"
#include "mir_test_framework/stubbed_server_configuration.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace mf = mir::frontend;
namespace mtf = mir_test_framework;
using namespace testing;

namespace
{
struct MockSessionMediatorReport : mf::SessionMediatorReport
{
    MockSessionMediatorReport()
    {
        EXPECT_CALL(*this, session_connect_called(_)).Times(AnyNumber());
        EXPECT_CALL(*this, session_disconnect_called(_)).Times(AnyNumber());

        // These are not needed for the 1st test, but they will be soon
        EXPECT_CALL(*this, session_create_surface_called(_)).Times(AnyNumber());
        EXPECT_CALL(*this, session_release_surface_called(_)).Times(AnyNumber());
        EXPECT_CALL(*this, session_next_buffer_called(_)).Times(AnyNumber());
    }

    MOCK_METHOD1(session_connect_called, void (std::string const&));
    MOCK_METHOD1(session_create_surface_called, void (std::string const&));
    MOCK_METHOD1(session_next_buffer_called, void (std::string const&));
    MOCK_METHOD1(session_release_surface_called, void (std::string const&));
    MOCK_METHOD1(session_disconnect_called, void (std::string const&));
    MOCK_METHOD2(session_start_prompt_session_called, void (std::string const&, pid_t));
    MOCK_METHOD2(session_add_prompt_provider_called, void (std::string const&, pid_t));
    MOCK_METHOD1(session_stop_prompt_session_called, void (std::string const&));

    void session_drm_auth_magic_called(const std::string&) override {};
    void session_configure_surface_called(std::string const&) override {};
    void session_configure_surface_cursor_called(std::string const&) override {};
    void session_configure_display_called(std::string const&) override {};
    void session_error(const std::string&, const char*, const std::string&) override {};
};

struct HostServerConfiguration : mtf::StubbedServerConfiguration
{
    virtual std::shared_ptr<mf::SessionMediatorReport>  the_session_mediator_report()
    {
        if (!mock_session_mediator_report)
            mock_session_mediator_report = std::make_shared<MockSessionMediatorReport>();

        return mock_session_mediator_report;
    }

    std::shared_ptr<MockSessionMediatorReport> mock_session_mediator_report;
};

struct FakeCommandLine
{
    static int const argc = 6;
    char const* argv[argc];

    FakeCommandLine(std::string const& host_socket)
    {
        char const** to = argv;
        for(auto from : { "--file", "NestedServer", "--host-socket", host_socket.c_str(), "--enable-input", "off"})
        {
            *to++ = from;
        }

        EXPECT_THAT(to - argv, Eq(argc)); // Check the array size matches parameter list
    }
};

struct NestedServerConfiguration : FakeCommandLine, public mir::DefaultServerConfiguration
{
    NestedServerConfiguration(std::string const& host_socket) :
        FakeCommandLine(host_socket),
        DefaultServerConfiguration(FakeCommandLine::argc, FakeCommandLine::argv)
    {
    }
};

struct NestedServer : mtf::InProcessServer, HostServerConfiguration
{
    virtual mir::DefaultServerConfiguration& server_config()
    {
        return *this;
    }
};
}

TEST_F(NestedServer, nested_platform_connects_and_disconnects)
{
    std::string const connection_string{new_connection()};
    NestedServerConfiguration nested_config(connection_string);

    InSequence seq;
    EXPECT_CALL(*mock_session_mediator_report, session_connect_called(_)).Times(1);
    EXPECT_CALL(*mock_session_mediator_report, session_disconnect_called(_)).Times(1);

    mir::run_mir(nested_config, [](mir::DisplayServer& server){server.stop();});
}

#ifdef DISABLED_UNTIL_REWRITTEN
//////////////////////////////////////////////////////////////////
// TODO the following tests were used in investigating lifetime issues.
// TODO they may not have much long term value, but decide that later

TEST(DisplayLeak, on_exit_display_objects_should_be_destroyed)
{
    struct MyServerConfiguration : mtf::TestingServerConfiguration
    {
        std::shared_ptr<mir::graphics::Display> the_display() override
        {
            auto const& temp = mtf::TestingServerConfiguration::the_display();
            my_display = temp;
            return temp;
        }

        std::weak_ptr<mir::graphics::Display> my_display;
    };

    MyServerConfiguration host_config;

    mir::run_mir(host_config, [](mir::DisplayServer& server){server.stop();});

    EXPECT_FALSE(host_config.my_display.lock()) << "after run_mir() exits the display should be released";
}

TEST_F(TestNestedMir, DISABLED_ON_ANDROID_AND_MESA(on_exit_display_objects_should_be_destroyed))
{
    struct MyNestedServerConfiguration : NestedServerConfiguration
    {
        // TODO clang says "error: inheriting constructors are not supported"
        // using NestedServerConfiguration::NestedServerConfiguration;
        MyNestedServerConfiguration(std::string const& host_socket) : NestedServerConfiguration(host_socket) {}

        std::shared_ptr<mir::graphics::Display> the_display() override
        {
            auto const& temp = NestedServerConfiguration::the_display();
            my_display = temp;
            return temp;
        }

        ~MyNestedServerConfiguration()
        {
            EXPECT_FALSE(my_display.lock()) << "after run_mir() exits the display should be released";
        }

        std::weak_ptr<mir::graphics::Display> my_display;
    };

    HostServerConfiguration host_config;
    ClientConfig<MyNestedServerConfiguration> client_config(host_config.the_socket_file());

    launch_server_process(host_config);
    launch_client_process(client_config);
}
#endif
