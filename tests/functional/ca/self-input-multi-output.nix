let
  single = derivation {
    __contentAddressed = true;
    name = "drv";
    system = "x86_64-linux";
    builder = "/bin/sh";
    args = [
      "-c"
      "echo same_content > $out"
    ];
  };
  multi = derivation {
    __contentAddressed = true;
    name = "drv";
    system = "x86_64-linux";
    builder = "/bin/sh";
    inherit single;
    outputs = [
      "out"
      "different"
    ];
    args = [
      "-c"
      "echo same_content > $out && echo other_content > $different"
    ];
  };
in
{
  inherit single multi;
}
