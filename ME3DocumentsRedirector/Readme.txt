ME3 Documents Redirector

This plugin acts like a proxy for the SHGetFolderPathW function from Shell32.dll, making the game use an alternate folder in the place of the current user's Documents folder.

The new folder is defined through a file called ME3DocumentsRedirector.txt, which must be placed in any valid folder for ASI plugins ("Mass Effect 3\Binaries\Win32" or "Mass Effect 3\Binaries\Win32\ASI").

The file should have only one line ("folder + new line", or just "folder", no quotes).

=========

Examples:

*** Assuming ME3 is installed in 'D:\Games\Mass Effect 3' ***

"" (nothing) -> game will read from/write to '%userprofile%\Documents\BioWare\Mass Effect 3' (plugin won't apply redirection if ME3DocumentsRedirector.txt is missing or, if it exists, is empty)

"Abc" -> game will read from/write to 'D:\Games\Mass Effect 3\Binaries\Win32\Abc\BioWare\Mass Effect 3'

"\Xyz" -> game will read from/write to 'D:\Xyz\BioWare\Mass Effect 3'

"E:" or "E:\" -> game will read from/write to 'E:\BioWare\Mass Effect 3'