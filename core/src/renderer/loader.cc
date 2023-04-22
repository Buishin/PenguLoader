#include "../internal.h"
#include <chrono>

// RENDERER PROCESS ONLY.

void LoadPlugins(cef_frame_t *frame, cef_v8context_t *context)
{
    auto pluginsDir = config::getPluginsDir();

    if (!utils::isDir(pluginsDir))
        return;

    using namespace std::chrono;
    auto current_time = system_clock::now();
    auto current_ms = duration_cast<milliseconds>(current_time.time_since_epoch()).count();

    int count = 0;
    std::wstring code = L"!(function () { ";
    code.append(L"window.__hookEvents?.(); ");

    // Scan plugins dir.
    for (const auto &name : utils::readDir(pluginsDir + L"\\*"))
    {
        // Skip name starts with underscore or dot.
        if (name[0] == '_' || name[0] == '.')
            continue;

        // Top-level JS file.
        if (utils::strEndWith(name, L".js") && utils::isFile(pluginsDir + L"\\" + name))
        {
            code.append(L"import(\"https://plugins/");
            code.append(name + L"?t=");
            code.append(std::to_wstring(current_ms));
            code.append(L"\"); ");

            count++;
        }
        // Sub-folder with index.
        else if (utils::isDir(pluginsDir + L"\\" + name) && utils::isFile(pluginsDir + L"\\" + name + L"\\index.js"))
        {
            code.append(L"import(\"https://plugins/");
            code.append(name + L"/index.js?t=");
            code.append(std::to_wstring(current_ms));
            code.append(L"\"); ");

            count++;
        }
    }

    code.append(L"})();");

    // Execute script.
    if (count > 0)
    {
        CefStr script{ code };
        frame->execute_java_script(frame, &script, &""_s, 1);
    }
}

bool HandlePlugins(const wstring &fn, const vector<cef_v8value_t *> &args, cef_v8value_t * &retval)
{
    if (fn == L"RequireFile")
    {
        if (args.size() > 0 && args[0]->is_string(args[0]))
        {
            string content{};
            CefScopedStr path_tmp{ args[0]->get_string_value(args[0]) };
            CefScopedStr path{ CefURIDecode(&path_tmp, true,
                static_cast<cef_uri_unescape_rule_t>(UU_SPACES | UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS)) };
            wstring _path{ path.str, path.length };

            size_t pos = _path.find(L"//");
            if (pos != string::npos)
                _path = _path.substr(pos + 2);

            if (_path.length() > 1 && _path[0] == L'/')
                _path = _path.substr(1);

            _path = config::getLoaderDir()
                .append(L"/").append(_path);

            if (utils::readFile(_path, content))
            {
                retval = CefV8Value_CreateString(&CefStr(content));
                return true;
            }
        }

        retval = CefV8Value_CreateNull();
        return true;
    }

    return false;
}