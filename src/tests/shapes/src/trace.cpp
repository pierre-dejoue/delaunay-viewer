#include <trace.h>

#include <filesystem>

std::filesystem::path stdutils::trace_folder_path()
{
    return STDUTILS_TRACE_FILE_FOLDER;
}
