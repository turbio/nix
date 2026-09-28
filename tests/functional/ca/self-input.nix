let
  first = derivation {
    __contentAddressed = true;
    name = "pkg";
    system = "x86_64-linux";
    builder = "/bin/sh";
    args = [
      "-c"
      "echo 'same contentt' > $out"
    ];
  };

  second = derivation {
    __contentAddressed = true;
    name = "pkg";
    system = "x86_64-linux";
    builder = "/bin/sh";
    args = [
      "-c"
      "echo 'same contentt' > $out"
    ];
    inherit first;
  };
in
{
  inherit first second;
}
