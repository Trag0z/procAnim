#pragma once
#include <shobjidl.h>
#include <codecvt>
#include "Util.h"

float length_squared(glm::vec2 v) {
    return v.x * v.x + v.y * v.y;
}

bool get_save_path(std::string& out_path,
                   cwstrptr_t filter_name,
                   cwstrptr_t filter_pattern,
                   cwstrptr_t default_extension) {
    HRESULT hr =
      CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SDL_assert_always(SUCCEEDED(hr));
    IFileSaveDialog* pFileSave;

    hr = CoCreateInstance(CLSID_FileSaveDialog,
                          NULL,
                          CLSCTX_ALL,
                          IID_IFileSaveDialog,
                          reinterpret_cast<void**>(&pFileSave));

    SDL_assert_always(SUCCEEDED(hr));

    if (filter_name && filter_pattern) {
        COMDLG_FILTERSPEC file_type = { filter_name, filter_pattern };
        pFileSave->SetFileTypes(1, &file_type);
    }

    if (default_extension) {
        pFileSave->SetDefaultExtension(default_extension);
    }

    // Show the Open dialog box.
    hr = pFileSave->Show(NULL);

    // Get the file name from the dialog box.
    if (!SUCCEEDED(hr)) { return false; }

    IShellItem* pItem;
    hr = pFileSave->GetResult(&pItem);

    SDL_assert_always(SUCCEEDED(hr));
    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    std::wstring w_string(pszFilePath);
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    out_path = converter.to_bytes(w_string);

    CoTaskMemFree(pszFilePath);

    SDL_assert_always(SUCCEEDED(hr));

    pItem->Release();
    pFileSave->Release();

    CoUninitialize();
    return true;
}

bool get_load_path(std::string& out_path,
                   cwstrptr_t filter_name,
                   cwstrptr_t filter_pattern) {
    HRESULT hr =
      CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SDL_assert_always(SUCCEEDED(hr));
    IFileOpenDialog* pFileOpen;

    hr = CoCreateInstance(CLSID_FileOpenDialog,
                          NULL,
                          CLSCTX_ALL,
                          IID_IFileOpenDialog,
                          reinterpret_cast<void**>(&pFileOpen));

    SDL_assert_always(SUCCEEDED(hr));

    if (filter_name && filter_pattern) {
        COMDLG_FILTERSPEC file_type = { filter_name, filter_pattern };
        pFileOpen->SetFileTypes(1, &file_type);
    }

    // Show the Open dialog box.
    hr = pFileOpen->Show(NULL);

    if (!SUCCEEDED(hr)) { return false; }

    // Get the file name from the dialog box.
    IShellItem* pItem;
    hr = pFileOpen->GetResult(&pItem);

    SDL_assert_always(SUCCEEDED(hr));
    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    std::wstring w_string(pszFilePath);
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    out_path = converter.to_bytes(w_string);

    CoTaskMemFree(pszFilePath);

    SDL_assert_always(SUCCEEDED(hr));

    pItem->Release();
    pFileOpen->Release();

    CoUninitialize();
    return true;
}