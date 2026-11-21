## run

+----------+--------+--------------------------+
| key      | type   | value                    |
+==========+========+==========================+
| epochs   | int    | 1                        |
+----------+--------+--------------------------+
| command  | string | stdio-client.sh          |
+----------+--------+--------------------------+
| samples  | string | stdio-client-samples.md  |
+----------+--------+--------------------------+
| output   | string | stdio-client-report.md   |
+----------+--------+--------------------------+
| baseline | string |                          |
+----------+--------+--------------------------+

## stdio client

| name | command | flag | input | streams |
|---|---|---|---|---|
| echo | echo | probe | | out |
| loop | head | -c 5 | probe | out |
| errorJoined | sh | -c "echo probe 1>&2" | | out+err |
| errorApart | sh | -c "echo probe 1>&2" | | out |
| early | true | | probe | out |

## toolchain

+---------+---------------------------+
| command | flag                      |
+=========+===========================+
| cast    | ../cast/spell.md --schema |
+---------+---------------------------+
