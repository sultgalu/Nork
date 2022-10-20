module Nork.Editor.Views;

import Nork.Editor;
import Nork.Core;

namespace Nork::Editor {
	Engine& View::GetEngine() { return Editor::Get().engine; }
	CommonData& View::GetCommonData() { return Editor::Get().data; }
}