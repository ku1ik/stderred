class Stderred < Formula
  desc "Colorize stderr output"
  homepage "https://github.com/ku1ik/stderred"
  license "MIT"
  head "https://github.com/ku1ik/stderred.git", branch: "master"

  depends_on "cmake" => :build

  define_method(:caveats) do
    [
      on_macos do
        <<~EOS
          To enable stderred for all commands in your shell, add this to your shell profile:
            export DYLD_INSERT_LIBRARIES="$(brew --prefix stderred)/lib/libstderred.dylib${DYLD_INSERT_LIBRARIES:+:$DYLD_INSERT_LIBRARIES}"
        EOS
      end,
      on_linux do
        <<~EOS
          To enable stderred for all commands in your shell, add this to your shell profile:
            export LD_PRELOAD="$(brew --prefix stderred)/lib/libstderred.so${LD_PRELOAD:+:$LD_PRELOAD}"
        EOS
      end,
    ].compact.first
  end

  def install
    system "cmake", "-S", "src", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"

    library_name = OS.mac? ? "libstderred.dylib" : "libstderred.so"

    lib.install "build/#{library_name}"
  end

  test do
    library_name = OS.mac? ? "libstderred.dylib" : "libstderred.so"
    preload_env_name = OS.mac? ? "DYLD_INSERT_LIBRARIES" : "LD_PRELOAD"

    assert_path_exists lib/library_name

    (testpath/"print_preload.c").write <<~C
      #include <stdio.h>
      #include <stdlib.h>

      int main(void) {
        const char *value = getenv("#{preload_env_name}");
        if (value == NULL) return 1;
        puts(value);
        return 0;
      }
    C

    system ENV.cc, "print_preload.c", "-o", "print_preload"

    output = shell_output(%Q(#{preload_env_name}="#{opt_lib/library_name}" #{testpath}/print_preload))
    assert_equal "#{opt_lib/library_name}\n", output
  end
end
