# 📊 Stats

| Stat | `containers` | matrices (`core`) | `representations` |
| --- | :---: | :---: | :---: |
| `activePixels()` | ✔️ | | |
| `boundingBox()` | ✔️ | | |
| `covariance()` | ✔️ | | |
| `density()` | ✔️ | ✔️ | ✔️ |
| `duration()` | ✔️ | ✔️ | ✔️ |
| `entropy()` | ✔️ | | |
| `fillRatio()` | ✔️ | | |
| `mean()` | ✔️ | | |
| `meanPoint()` | ✔️ | | |
| `meanTime()` | ✔️ | | |
| `midTime()` | ✔️ | ✔️ | ✔️ |
| `peak()` | ✔️ | | |
| `polarityRatio()` | ✔️ | | |
| `rate()` | ✔️ | ✔️ | ✔️ |

**Notes**

- `Queue` and `ConcurrentQueue` offer no stats, since they cannot be iterated.
- `Grid` offers no stats of its own. Each cell offers those of the container it is made of.
- Matrices only account for the events passed to `updateStats()`, which `insert()` and `emplace()` never call.
- Representations account for the events inserted since the last `clear()`.
- `StatsContainer` offers every stat without storing the events, so its events cannot be retrieved.
