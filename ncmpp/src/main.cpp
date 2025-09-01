
#include "cmdline.h"
#include "app_config.h"
#include "app_logic.h"
#include <thread>

int main(int argc, char* argv[]) {
    cmdline::parser cmd;
    cmd.set_program_name("ncmpp");
    cmd.add<unsigned int>("threads", 't', "Max count of unlock threads.", false, std::thread::hardware_concurrency());
    cmd.add("showtime", 's', "Shows how long it took to unlock everything.");
    cmd.add<std::string>("input", 'i', "Path to a text file containing a list of input .ncm files.", false, "");
    cmd.add<std::string>("output", 'o', "Path to a text file containing a list of output files or a directory for fallback mode.", false, "unlocked");
    cmd.add("help", 'h', "Print this message.");

    cmd.parse_check(argc, argv);

    app_config config;
    config.thread_count = cmd.get<unsigned int>("threads");
    config.show_time = cmd.exist("showtime");
    config.input_file_list = cmd.get<std::string>("input");

    // Determine if output is a file or directory
    std::string output_path_str = cmd.get<std::string>("output");
    if (!config.input_file_list.empty()) {
        config.output_file_list = output_path_str;
    } else {
        config.output_dir = output_path_str;
    }

    ncm_app app(config);
    return app.run();
}
