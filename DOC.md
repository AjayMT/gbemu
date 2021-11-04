
Implemented CPU instructions:

| Assembly Text | Encoding | Length | Clock Cycles | Description |
| ------------- | -------- | ------ | ------------ | ----------- |
| `ld r d16`    | `0x{r} {d16}` | 3 | 12           | Load 16-bit immediate `{d16}` into register `{r}` |
| `ld r1 r2`    | `0x{r1}{r2}`  | 1 | 4            | Load register `{r2}` into `{r1}` |
| `ld r1 (hl)`  | `0x{r1}E` or `0x{r1}6` | 1 | 8            | Load the data pointed to by `hl` into `{r1}` |
| `add a r`     | `0x8{r}`      | 1 | 4            | Add `a` to `{r}` and store in `a` |
