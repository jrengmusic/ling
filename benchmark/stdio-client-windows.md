## run

+----------+--------+----------------------------------+
| key      | type   | value                            |
+==========+========+==================================+
| epochs   | int    | 1                                |
+----------+--------+----------------------------------+
| command  | string | stdio-client.cmd                 |
+----------+--------+----------------------------------+
| samples  | string | stdio-client-samples.md          |
+----------+--------+----------------------------------+
| output   | string | stdio-client-windows-report.md   |
+----------+--------+----------------------------------+
| baseline | string |                                  |
+----------+--------+----------------------------------+

## stdio client

| name | command | flag | input | streams |
|---|---|---|---|---|
| echo | cmd | /c echo probe | | out |
| loop | powershell | -NoProfile -Command $b=[char[]]::new(5);[void][Console]::In.Read($b,0,5);[Console]::Out.Write($b) | probe | out |
| errorJoined | cmd | /c "1>&2 echo probe" | | out+err |
| errorApart | cmd | /c "1>&2 echo probe" | | out |
| early | cmd | /c exit 0 | probe | out |

## toolchain

+---------+---------------------------+
| command | flag                      |
+=========+===========================+
| cast    | ../cast/spell.md --schema |
+---------+---------------------------+
