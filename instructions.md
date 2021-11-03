
Implemented CPU instructions:

| Assembly Text | Encoding | Clock Cycles | Description |
| ------------- | -------- | ------------ | ----------- |
| ld r d16      | 0x{r} {d16} | 12        | Load 16-bit immediate {d16} into register {r} |
| ld r1 r2      | 0x{r1}{r2}  | 4         | Load register {r2} into {r1} |
