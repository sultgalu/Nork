export module Nork.FileDialog;

export namespace Nork::FileDialog
{
	enum class EngineFileTypes
	{
		None = 0,
		Shader = 1 << 0,
		_3D = 1 << 1,
		Image = 1 << 2,
		Json = 1 << 3,
		glTF = 1 << 4,
	};

	inline EngineFileTypes operator|(EngineFileTypes a, EngineFileTypes b)
	{
		return static_cast<EngineFileTypes>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline EngineFileTypes operator&(EngineFileTypes a, EngineFileTypes b)
	{
		return static_cast<EngineFileTypes>(static_cast<int>(a) & static_cast<int>(b));
	}

	std::string OpenFile(EngineFileTypes fileType, std::wstring title = std::wstring(), std::wstring okButton = std::wstring());
	std::string SaveFile(EngineFileTypes fileType, std::wstring title = std::wstring(), std::wstring okButton = std::wstring());
}