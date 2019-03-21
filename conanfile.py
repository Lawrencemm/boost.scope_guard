from conans import ConanFile, CMake, tools


class BoostscopeguardConan(ConanFile):
    name = "boost_scope_guard"
    version = "1.0"
    requires = 'boost_config/1.66.0@bincrafters/stable'
    exports_sources = "include/*"
    no_copy_source = True

    def package(self):
        self.copy("*.hpp")
