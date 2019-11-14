// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

#include "ConptyConnection.g.h"
#include "../inc/cppwinrt_utils.h"

namespace wil
{
    // These belong in WIL upstream, so when we reingest the change that has them we'll get rid of ours.
    using unique_pseudoconsole_handle = wil::unique_any<HPCON, decltype(&::ClosePseudoConsole), ::ClosePseudoConsole>;
}

namespace winrt::Microsoft::Terminal::TerminalConnection::implementation
{
    struct ConptyConnection : ConptyConnectionT<ConptyConnection>
    {
        ConptyConnection(const hstring& cmdline, const hstring& startingDirectory, const hstring& startingTitle, const uint32_t rows, const uint32_t cols, const guid& guid);

        void Start();
        void WriteInput(hstring const& data);
        void Resize(uint32_t rows, uint32_t columns);
        void Close();

        winrt::guid Guid() const noexcept;

        ConnectionState State() const noexcept { return _state; }

        WINRT_CALLBACK(TerminalOutput, TerminalOutputHandler);
        TYPED_EVENT(StateChanged, ITerminalConnection, IInspectable);

    private:
        HRESULT _LaunchAttachedClient() noexcept;
        void _ClientTerminated() noexcept;
        bool _transitionToState(const ConnectionState state) noexcept;

        uint32_t _initialRows{};
        uint32_t _initialCols{};
        hstring _commandline;
        hstring _startingDirectory;
        hstring _startingTitle;
        guid _guid{}; // A unique session identifier for connected client

        std::atomic<ConnectionState> _state{ ConnectionState::NotConnected };
        mutable std::mutex _stateMutex;

        bool _recievedFirstByte{ false };
        std::chrono::high_resolution_clock::time_point _startTime{};

        wil::unique_hfile _inPipe; // The pipe for writing input to
        wil::unique_hfile _outPipe; // The pipe for reading output from
        wil::unique_handle _hOutputThread;
        wil::unique_process_information _piClient;
        wil::unique_pseudoconsole_handle _hPC;
        wil::unique_threadpool_wait _clientExitWait;

        DWORD _OutputThread();
    };
}

namespace winrt::Microsoft::Terminal::TerminalConnection::factory_implementation
{
    struct ConptyConnection : ConptyConnectionT<ConptyConnection, implementation::ConptyConnection>
    {
    };
}
