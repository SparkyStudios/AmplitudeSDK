#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <CLI/CLI.hpp>
#include <iostream>
#include <random>

#include <cmrc/cmrc.hpp>

using namespace SparkyStudios::Audio::Amplitude;

CMRC_DECLARE(ampm);

static constexpr char kProjectPathAttenuators[] = "attenuators";
static constexpr char kProjectPathCollections[] = "collections";
static constexpr char kProjectPathEffects[] = "effects";
static constexpr char kProjectPathEvents[] = "events";
static constexpr char kProjectPathPipelines[] = "pipelines";
static constexpr char kProjectPathRTPC[] = "rtpc";
static constexpr char kProjectPathSoundBanks[] = "soundbanks";
static constexpr char kProjectPathSounds[] = "sounds";
static constexpr char kProjectPathSwitchContainers[] = "switch_containers";
static constexpr char kProjectPathSwitches[] = "switches";

static constexpr uint32_t kProjectVersion = 1;

/// Credits: https://stackoverflow.com/a/13059195
/// https://stackoverflow.com/questions/13059091/
struct membuf : std::streambuf
{
    membuf(char const* base, size_t size)
    {
        char* p(const_cast<char*>(base));
        this->setg(p, p, p + size);
    }
    ~membuf() override = default;
};

/// Credits: https://stackoverflow.com/a/13059195
/// https://stackoverflow.com/questions/13059091/
struct memstream
    : virtual membuf
    , std::istream
{
    memstream(char const* base, char* const end)
        : membuf(base, reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(base))
        , std::istream(static_cast<std::streambuf*>(this))
    {}

    memstream(char const* base, size_t size)
        : membuf(base, size)
        , std::istream(static_cast<std::streambuf*>(this))
    {}
};

static std::string SnakeCase(const std::string& str)
{
    std::string result;
    result += static_cast<char>(std::tolower(str[0]));

    for (size_t i = 1, l = str.length(); i < l; i++)
    {
        if (const char ch = str[i]; std::isupper(ch))
        {
            result += '_';
            result += static_cast<char>(std::tolower(ch));
        }
        else if (ch == '-' || ch == ' ')
        {
            result += '_';
        }
        else
        {
            result += ch;
        }
    }

    return result;
}

struct InitProjectOptions
{
    AmString name;
    AmString templateName = "empty";
    AmString directory;
};

struct CreateSourceOptions
{
    AmObjectID id = std::random_device{}();
    AmString name;
    AmReal32 gain = 1.0f;
    AmReal32 pitch = 1.0f;
    AmBusID bus = 1;
    AmReal32 priority = 1.0f;
    bool stream = false;
    AmString scope = "World";
    AmString spatialization = "None";
    AmEffectID effect = 0;
    AmAttenuationID attenuation = 0;
    AmString fader = "Linear";
    AmString path;
};

struct ImportSourceOptions : public CreateSourceOptions
{
    AmString fileName;
};

struct AppContext
{
    std::filesystem::path exeDirectory;

    CLI::App mainApp;

    CLI::App* projectApp{ nullptr };
    CLI::App* projectNewApp{ nullptr };

    CLI::App* assetsApp{ nullptr };
    CLI::App* assetsImportApp{ nullptr };

    cmrc::embedded_filesystem resourcesFS;

    InitProjectOptions initProjectOptions;
    CreateSourceOptions createSourceOptions;
    ImportSourceOptions importSourceOptions;

    static void fillCommonSourceOptions(CLI::App* app, CreateSourceOptions& options)
    {
        app->add_option("--id", options.id, "Source ID.");
        app->add_option("--name", options.id, "Source name.")->required();
        app->add_option("--gain", options.gain, "Gain value.")->default_val(1.0f);
        app->add_option("--pitch", options.pitch, "Pitch value.")->default_val(1.0f);
        app->add_option("--bus", options.bus, "Bus ID.")->default_val(1);
        app->add_option("--priority", options.priority, "Source priority.")->default_val(1.0f);
        app->add_flag("--stream", options.stream, "Volume value.")->default_val(false);
        app->add_option("--scope", options.scope, "Source scope.")->default_str("World");
        app->add_option("--spatialization", options.spatialization, "Spatialization mode.")->default_str("None");
        app->add_option("--attenuation", options.attenuation, "Attenuation value.")->default_val(0);
        app->add_option("--fader", options.fader, "Fader name.")->default_str("Linear");
    }

    AppContext()
        : mainApp("Amplitude Project Manager", "ampm")
        , resourcesFS(cmrc::ampm::get_filesystem())
    {
        mainApp.require_subcommand(1);

        initProjectApp();
        initAssetsApp();
    }

    int run(int argc, char** argv)
    {
        exeDirectory = weakly_canonical(std::filesystem::path(argv[0])).parent_path();

        CLI11_PARSE(mainApp, argc, argv);

        if (projectNewApp->parsed())
            return runProjectNew();

        if (assetsImportApp->parsed())
            return runImportSource();

        return EXIT_SUCCESS;
    }

    [[nodiscard]] int runProjectNew() const
    {
        // Create project directory
        const auto projectDir = std::filesystem::path(initProjectOptions.directory) / initProjectOptions.name;
        if (exists(projectDir))
        {
            std::cerr << "Error: Project directory already exists: " << projectDir.string() << std::endl;
            return EXIT_FAILURE;
        }

        std::filesystem::create_directories(projectDir);

        std::cout << "Initializing project '" << initProjectOptions.name << "' using template '" << initProjectOptions.templateName
                  << "' in directory '" << projectDir.string() << "'." << std::endl;

        // Copy template files to project directory
        if (initProjectOptions.templateName != "empty")
        {
            const auto templateDir = exeDirectory / "templates" / initProjectOptions.templateName;
            if (!exists(templateDir))
            {
                std::cerr << "Error: Template directory not found: " << templateDir.string() << std::endl;
                return EXIT_FAILURE;
            }

            // Copy template files to project directory
            copy(
                templateDir, projectDir / "sources",
                std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
        }
        else
        {
            // Create project "sources" directories
            create_directories(projectDir / "sources" / kProjectPathAttenuators);
            create_directories(projectDir / "sources" / kProjectPathCollections);
            create_directories(projectDir / "sources" / kProjectPathEffects);
            create_directories(projectDir / "sources" / kProjectPathEvents);
            create_directories(projectDir / "sources" / kProjectPathPipelines);
            create_directories(projectDir / "sources" / kProjectPathRTPC);
            create_directories(projectDir / "sources" / kProjectPathSoundBanks);
            create_directories(projectDir / "sources" / kProjectPathSounds);
            create_directories(projectDir / "sources" / kProjectPathSwitchContainers);
            create_directories(projectDir / "sources" / kProjectPathSwitches);

            // Create default config file
            {
                auto configFile = resourcesFS.open("resources/default.config.json");
                auto is = memstream(const_cast<char*>(configFile.begin()), const_cast<char*>(configFile.end()));

                std::ofstream projectFile(projectDir / "sources" / "pc.config.json");
                projectFile << is.rdbuf();
                projectFile.flush();
            }

            // Create default buses file
            {
                auto configFile = resourcesFS.open("resources/default.buses.json");
                auto is = memstream(const_cast<char*>(configFile.begin()), const_cast<char*>(configFile.end()));

                std::ofstream projectFile(projectDir / "sources" / "pc.buses.json");
                projectFile << is.rdbuf();
                projectFile.flush();
            }

            // Create default pipeline file
            {
                auto configFile = resourcesFS.open("resources/default.pipeline.json");
                auto is = memstream(const_cast<char*>(configFile.begin()), const_cast<char*>(configFile.end()));

                std::ofstream projectFile(projectDir / "sources" / kProjectPathPipelines / "default.json");
                projectFile << is.rdbuf();
                projectFile.flush();
            }
        }

        // Create project "build" directory
        create_directories(projectDir / "build");

        // Create project "data" directory
        create_directories(projectDir / "data");

        // Create project "plugins" directory
        create_directories(projectDir / "plugins");

        // Create project file
        std::ofstream projectFile(projectDir / ".amproject");
        if (!projectFile.is_open())
        {
            std::cerr << "Error: Failed to create project file in directory: " << projectDir.string() << std::endl;
            return EXIT_FAILURE;
        }

        projectFile << R"({"name": ")"
            << initProjectOptions.name
            << R"(", "default_configuration": "pc.config.amconfig", "sources_dir": "sources", "data_dir": "data", "build_dir": "build", "version": )"
            << kProjectVersion
            << R"( })";
        projectFile.flush();

        std::cout << "Project '" << initProjectOptions.name << "' initialized successfully" << std::endl;

        return EXIT_SUCCESS;
    }

    [[nodiscard]] int runImportSource() const
    {
        // Implement import source logic here
        std::cout << "Importing source..." << std::endl;

        // Return success or failure status
        return EXIT_SUCCESS;
    }

    void initProjectApp()
    {
        const auto templateNameValidator = CLI::Validator(
            [this](const std::string& input) -> std::string
            {
                std::vector<std::string> validTemplates = { "empty" };
                if (const auto templatesDir = exeDirectory / "templates"; exists(templatesDir))
                    for (const std::filesystem::directory_iterator it(templatesDir); const auto& entry : it)
                        if (entry.is_directory())
                            validTemplates.push_back(entry.path().filename().string());

                if (std::ranges::find(validTemplates, input) == validTemplates.end())
                    return "The provided template name is not valid. Valid options are: " + CLI::detail::join(validTemplates, ", ") + ".";

                return "";
            },
            "TEMPLATE_NAME", "TEMPLATE_NAME");

        const auto projectNameTransformer = CLI::Validator(
            [](std::string& input) -> std::string
            {
                input = SnakeCase(input);
                return {};
            },
            "PROJECT_NAME", "PROJECT_NAME");

        projectApp = mainApp.add_subcommand("project", "Manage Amplitude projects");
        projectNewApp = projectApp->add_subcommand("new", "Initialize a new project");

        projectNewApp->add_option("-n,--name", initProjectOptions.name, "Project name.")->required()->transform(projectNameTransformer);

        projectNewApp
            ->add_option(
                "-t,--template", initProjectOptions.templateName, "Project template. If not set, defaults to the 'empty' template.")
            ->default_str("empty")
            ->check(templateNameValidator, "TEMPLATE_NAME");

        projectNewApp
            ->add_option(
                "-o,--output", initProjectOptions.directory, "Project destination directory. If not set, defaults to current directory.")
            ->default_str(std::filesystem::current_path().string());
    }

    void initAssetsApp()
    {
        assetsApp = mainApp.add_subcommand("assets", "Manage Amplitude assets");
        assetsImportApp = assetsApp->add_subcommand("import", "Import new source asset from external sound files");

        assetsImportApp->add_option("-i,--input", importSourceOptions.fileName, "Input file path.")->required();

        fillCommonSourceOptions(assetsImportApp, importSourceOptions);
    }
};

int main(int argc, char** argv)
{
    AppContext app;
    return app.run(argc, argv);
}
