#include "Sender.h"

#include <Windows.h>
#include <winhttp.h>

#include <string>

#pragma comment(lib, "Winhttp.lib")

Sender::Sender(
    const std::string& host,
    int port,
    const std::string& path)
    : host(host),
    port(port),
    path(path)
{
}

bool Sender::send(const std::string& json)
{
    std::wstring wideHost(
        host.begin(),
        host.end()
    );

    std::wstring widePath(
        path.begin(),
        path.end()
    );

    HINTERNET session = WinHttpOpen(
        L"SystemMonitoringAgent/1.0",
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (session == nullptr)
    {
        return false;
    }

    HINTERNET connection = WinHttpConnect(
        session,
        wideHost.c_str(),
        static_cast<INTERNET_PORT>(port),
        0
    );

    if (connection == nullptr)
    {
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"POST",
        widePath.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0
    );

    if (request == nullptr)
    {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    WinHttpSetTimeouts(
        request,
        1000,
        1000,
        1000,
        1000
    );

    const wchar_t* headers =
        L"Content-Type: application/json\r\n";

    BOOL result = WinHttpSendRequest(
        request,
        headers,
        static_cast<DWORD>(-1L),
        (LPVOID)json.data(),
        static_cast<DWORD>(json.size()),
        static_cast<DWORD>(json.size()),
        0
    );

    if (!result)
    {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);

        return false;
    }

    result = WinHttpReceiveResponse(
        request,
        nullptr
    );

    if (!result)
    {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);

        return false;
    }

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);

    result = WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_STATUS_CODE |
        WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusCodeSize,
        WINHTTP_NO_HEADER_INDEX
    );

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    if (!result)
    {
        return false;
    }

    return statusCode >= 200 && statusCode < 300;
}

bool Sender::isAvailable()
{
    std::wstring wideHost(
        host.begin(),
        host.end()
    );

    std::wstring widePath(
        path.begin(),
        path.end()
    );

    HINTERNET session = WinHttpOpen(
        L"SystemMonitoringAgent/1.0",
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (session == nullptr)
    {
        return false;
    }

    HINTERNET connection = WinHttpConnect(
        session,
        wideHost.c_str(),
        static_cast<INTERNET_PORT>(port),
        0
    );

    if (connection == nullptr)
    {
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        widePath.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0
    );

    if (request == nullptr)
    {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    BOOL result = WinHttpSendRequest(
        request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        nullptr,
        0,
        0,
        0
    );

    if (!result)
    {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    result = WinHttpReceiveResponse(
        request,
        nullptr
    );

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return result == TRUE;
}