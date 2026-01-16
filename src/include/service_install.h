#ifndef SERVICE_INSTALL_H
#define SERVICE_INSTALL_H

#include <windows.h>

BOOL InstallService();
BOOL UninstallService();
void ShowServiceStatus();

#endif // SERVICE_INSTALL_H
