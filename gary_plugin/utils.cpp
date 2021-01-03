void ShowErrorMessageBox(const char* message)
{
	int msgboxID = MessageBox(
		NULL,
		message,
		"Error",
		MB_ICONWARNING | MB_OK
	);
}