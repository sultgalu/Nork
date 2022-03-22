#include "../FileDialog.h"

#include <windows.h>
#include <shobjidl.h> 

namespace Nork::FileDialog
{
    const static COMDLG_FILTERSPEC shaderExtensions[]
    {
        COMDLG_FILTERSPEC{L"Shaders", L"*.shader"},
        COMDLG_FILTERSPEC{L"GLSL", L"*.glsl"},
    };

    const static COMDLG_FILTERSPEC _3DExtensions[]
    {
        COMDLG_FILTERSPEC{L"3D as FBX", L"*.fbx;*.obj"},
        //COMDLG_FILTERSPEC{L"3D as OBJ", L"*.obj"},
        COMDLG_FILTERSPEC{L"3D as COLLADA", L"*.dae"},
        COMDLG_FILTERSPEC{L"3D as BLENDER", L"*.blend"},
    };
    const static COMDLG_FILTERSPEC ImageExtensions[]
    {
        COMDLG_FILTERSPEC{L"3D as FBX", L"*.png;*.jpg"},
    };

    static std::vector<COMDLG_FILTERSPEC> GetExtensions(EngineFileTypes types)
    {
        std::vector<COMDLG_FILTERSPEC> result;
        if ((types & EngineFileTypes::Shader) != EngineFileTypes::None)
        {
            for (int i = 0; i < ARRAYSIZE(shaderExtensions); i++)
            {
                result.push_back(shaderExtensions[i]);
            }
        }
        if ((types & EngineFileTypes::_3D) != EngineFileTypes::None)
        {
            for (int i = 0; i < ARRAYSIZE(_3DExtensions); i++)
            {
                result.push_back(_3DExtensions[i]);
            }
        }
        if ((types & EngineFileTypes::Image) != EngineFileTypes::None)
        {
            for (int i = 0; i < ARRAYSIZE(ImageExtensions); i++)
            {
                result.push_back(ImageExtensions[i]);
            }
        }

        return result;
    }

    std::string OpenFile(EngineFileTypes fileType, std::wstring title, std::wstring okButton)
    {
        std::string result;

        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
            COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
        {
            IFileOpenDialog* pFileOpen;

            if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen))))
            {
                if (!title.empty())
                    pFileOpen->SetTitle(title.c_str());
                if (!okButton.empty())
                    pFileOpen->SetOkButtonLabel(okButton.c_str());

                std::vector<COMDLG_FILTERSPEC> filters = GetExtensions(fileType);
                if (!filters.empty())
                {
                    pFileOpen->SetFileTypes(filters.size(), filters.data());
                    pFileOpen->SetFileTypeIndex(0);
                }
                pFileOpen->SetDefaultExtension(L"*.txt");

                if (SUCCEEDED(pFileOpen->Show(NULL)))
                {
                    IShellItem* pItem;
                    hr = pFileOpen->GetResult(&pItem);
                    if (SUCCEEDED(hr))
                    {
                        PWSTR pszFilePath;

                        if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                        {
                            int i = 0;
                            while (pszFilePath[i] != '\0')
                                result.push_back(pszFilePath[i++]);
                        }
                        pItem->Release();
                    }
                }
                pFileOpen->Release();
            }
            CoUninitialize();
        }
        return result;
    }

    std::string SaveFile(EngineFileTypes fileType, std::wstring title, std::wstring okButton)
    {
        std::string result;

        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
            COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
        {
            IFileSaveDialog* pFileSave;

            if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave))))
            {
                if (!title.empty())
                    pFileSave->SetTitle(title.c_str());
                if (!okButton.empty())
                    pFileSave->SetOkButtonLabel(okButton.c_str());

                std::vector<COMDLG_FILTERSPEC> filters = GetExtensions(fileType);
                if (!filters.empty())
                {
                    pFileSave->SetFileTypes(filters.size(), filters.data());
                    pFileSave->SetFileTypeIndex(0);
                }
                pFileSave->SetDefaultExtension(L"*.txt");

                if (SUCCEEDED(pFileSave->Show(NULL)))
                {
                    IShellItem* pItem;
                    hr = pFileSave->GetResult(&pItem);
                    if (SUCCEEDED(hr))
                    {
                        PWSTR pszFilePath;

                        if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                        {
                            int i = 0;
                            while (pszFilePath[i] != '\0')
                                result.push_back(pszFilePath[i++]);
                        }
                        pItem->Release();
                    }
                }
                pFileSave->Release();
            }
            CoUninitialize();
        }
        return result;
    }
}