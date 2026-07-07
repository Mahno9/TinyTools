#pragma once
#include <QString>
#include <QtGlobal>

namespace Logging {

// Installs a Qt message handler writing to filePath (directories are created).
// Rotation: if the existing file exceeds maxBytes it is renamed to "<path>.old"
// (one previous generation is kept).
// QtDebugMsg lines are dropped unless the TINYTOOLS_DEBUG env var is set.
bool init(const QString &filePath, qint64 maxBytes = 5 * 1024 * 1024);

// Restores the previous message handler and closes the log file.
void shutdown();

bool isDebugEnabled();

} // namespace Logging
