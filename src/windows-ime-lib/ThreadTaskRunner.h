#pragma once

class TaskRunner
{
public:
    // This class need to be instantiated on the thread that tasks are consumed.
    TaskRunner()
    {
        if (s_winClass == 0)
        {
            WNDCLASSEXW wcex = {};
            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = StaticWinProc;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = Global::dllInstanceHandle;
            wcex.hIcon = nullptr;
            wcex.hCursor = nullptr;
            wcex.hbrBackground = nullptr;
            wcex.lpszMenuName = nullptr;
            wcex.lpszClassName = s_winClassName;
            wcex.hIconSm = nullptr;
            s_winClass = RegisterClassExW(&wcex);

            s_messageId = RegisterWindowMessage(s_eventMessageName);
        }

        if (!m_finishEvent)
        {
            m_finishEvent.reset(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        }

        if (!m_hwnd)
        {
            m_hwnd.reset(CreateWindowEx(
                0,
                s_winClassName,
                s_windowName,
                WS_POPUP,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                HWND_MESSAGE, nullptr, Global::dllInstanceHandle, this));
        }
    }

    ~TaskRunner()
    {
        m_hwnd.reset();
    }

    void RequestTask(const std::function<void(void)>& task)
    {
        PostMessage(m_hwnd.get(), s_messageId, 0, reinterpret_cast<LPARAM>(&task));
        WaitForSingleObject(m_finishEvent.get(), INFINITE);
    }

private:
    static LRESULT CALLBACK StaticWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_CREATE)
        {
            const CREATESTRUCT* cs = reinterpret_cast<const CREATESTRUCT*>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        }
        else if (message == WM_DESTROY)
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        }
        else if (message == s_messageId)
        {
            if (auto _this = reinterpret_cast<TaskRunner*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
            {
                const auto task = reinterpret_cast<const std::function<void(void)>*>(lParam);
                try { (*task)(); } CATCH_LOG();
                SetEvent(_this->m_finishEvent.get());
                return TRUE;
            }
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    static constexpr wchar_t s_winClassName[] = L"thread-task-runner-class";
    static constexpr wchar_t s_windowName[] = L"thread-task-runner-windowname";
    static constexpr wchar_t s_eventMessageName[] = L"thread-task-request-message";
    static inline ATOM s_winClass = 0;
    static inline UINT s_messageId = 0;
    wil::unique_hwnd m_hwnd;
    wil::unique_handle m_finishEvent;
};

class ThreadTaskRunner
{
public:
    ThreadTaskRunner() {}
    ~ThreadTaskRunner()
    {
        if (m_thread)
        {
            m_requestedTask = nullptr;
            SetEvent(m_taskRequested.get());
            WaitForSingleObject(m_thread.get(), INFINITE);
        }
    }
    void RunOnThread(const std::function<void(void)>& task)
    {
        EnsureInitialized();
        m_requestedTask = &task;
        SetEvent(m_taskRequested.get());
        WaitForSingleObject(m_taskFinished.get(), INFINITE);
        m_requestedTask = nullptr;
    }

private:
    void EnsureInitialized()
    {
        if (!m_thread)
        {
            m_taskRequested.reset(CreateEvent(nullptr, FALSE, FALSE, nullptr));
            m_taskFinished.reset(CreateEvent(nullptr, FALSE, FALSE, nullptr));
            m_thread.reset(CreateThread(nullptr, 0, ThreadFunction, this, 0, &m_threadId));
        }
    }

    static DWORD WINAPI ThreadFunction(void* lpParameter)
    {
        auto _this = reinterpret_cast<ThreadTaskRunner*>(lpParameter);
        const auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

        for (;;)
        {
            WaitForSingleObject(_this->m_taskRequested.get(), INFINITE);
            if (const auto task = _this->m_requestedTask)
            {
                try
                {
                    (*task)();
                }
                CATCH_LOG();
                SetEvent(_this->m_taskFinished.get());
            }
            else
            {
                return 0;
            }
        }
    }

private:
    const std::function<void(void)>* m_requestedTask = nullptr;
    wil::unique_handle m_taskRequested;
    wil::unique_handle m_taskFinished;
    wil::unique_handle m_thread;
    DWORD m_threadId = 0;
};


