#pragma once
#include "pch.h"
#include "Util.h"

void printGlm(const glm::vec4& v) {
    printf("%.4f, %.4f, %.4f, %.4f\n", v[0], v[1], v[2], v[3]);
}

void printGlm(const glm::mat4& m) {
    printf("%.4f, %.4f, %.4f, %.4f\n%.4f, %.4f, %.4f, %.4f\n%.4f, "
           "%.4f, %.4f, %.4f\n%.4f, %.4f, %.4f, %.4f\n",
           m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1],
           m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3],
           m[2][3], m[3][3]);
}

bool get_save_path(char*& path, cwstrptr_t filter_name,
                   cwstrptr_t filter_pattern, cwstrptr_t default_extension) {
    HRESULT hr =
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SDL_assert_always(SUCCEEDED(hr));
    IFileSaveDialog* pFileSave;

    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                          IID_IFileSaveDialog,
                          reinterpret_cast<void**>(&pFileSave));

    SDL_assert_always(SUCCEEDED(hr));

    if (filter_name && filter_pattern) {
        COMDLG_FILTERSPEC file_type = {filter_name, filter_pattern};
        pFileSave->SetFileTypes(1, &file_type);
    }

    if (default_extension) {
        pFileSave->SetDefaultExtension(default_extension);
    }

    // Show the Open dialog box.
    hr = pFileSave->Show(NULL);

    // Get the file name from the dialog box.
    if (!SUCCEEDED(hr))
        return false;

    IShellItem* pItem;
    hr = pFileSave->GetResult(&pItem);

    SDL_assert_always(SUCCEEDED(hr));
    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    // Copy path to parameter, delete if there is already something there
    if (path) {
        delete[] path;
    }

    size_t length = wcslen(pszFilePath) + 1;
    path = new char[length];

    wcstombs_s(nullptr, path, length, pszFilePath, length);

    CoTaskMemFree(pszFilePath);

    SDL_assert_always(SUCCEEDED(hr));

    pItem->Release();
    pFileSave->Release();

    CoUninitialize();
    return true;
}

bool get_load_path(char*& path, cwstrptr_t filter_name,
                   cwstrptr_t filter_pattern) {
    HRESULT hr =
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SDL_assert_always(SUCCEEDED(hr));
    IFileOpenDialog* pFileOpen;

    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                          IID_IFileOpenDialog,
                          reinterpret_cast<void**>(&pFileOpen));

    SDL_assert_always(SUCCEEDED(hr));

    if (filter_name && filter_pattern) {
        COMDLG_FILTERSPEC file_type = {filter_name, filter_pattern};
        pFileOpen->SetFileTypes(1, &file_type);
    }

    // Show the Open dialog box.
    hr = pFileOpen->Show(NULL);

    if (!SUCCEEDED(hr)) {
        return false;
    }

    // Get the file name from the dialog box.
    IShellItem* pItem;
    hr = pFileOpen->GetResult(&pItem);

    SDL_assert_always(SUCCEEDED(hr));
    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    if (path) {
        delete[] path;
    }

    size_t length = wcslen(pszFilePath) + 1;
    path = new char[length];

    wcstombs_s(nullptr, path, length, pszFilePath, length);

    CoTaskMemFree(pszFilePath);

    SDL_assert_always(SUCCEEDED(hr));

    pItem->Release();
    pFileOpen->Release();

    CoUninitialize();
    return true;
}