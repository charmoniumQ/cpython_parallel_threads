import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv('count.csv', index_col=(0, 1), skiprows=1)

base = df.loc['single']['user_time'].mean()

cmap = {
    'pat': 'green',
    'thread': 'blue',
    'proc': 'red',
    'single': 'black',
}

for type_ in ['runtime', 'speedup']:
    for time in ['total']:
        fig = plt.figure()
        ax = fig.gca()
        ax.set_title(f'{type_.capitalize()}')
        if type_ == 'runtime':
            ax.set_ylabel('Runtime (s)')
        else:
            ax.set_ylabel('Speedup (x non-parallel runtime)')
        ax.set_xlabel('Parallelism (CPUs)')

        progs = list(df.index.levels[0])
        progs.remove('single')

        for prog in progs:
            xs = np.array(sorted(list(set(df.loc[prog].index))))
            xs = xs[xs < 72]
            ys = np.zeros_like(xs)
            yerr = np.zeros_like(xs)
            for i, x in enumerate(xs):
                s = np.array(df.loc[prog].loc[x][f'{time}_time'])
                if type_ == 'speedup':
                    s = base / s
                ys[i] = s.mean()
                yerr[i] = np.clip(s.std(ddof=1), 0, ys[i])
            ax.errorbar(xs, ys, yerr=yerr, label=prog, color=cmap[prog])

        # if type_ == 'runtime':
        #     prog = 'single'
        #     ax.errorbar(xs, [base] * len(xs), yerr=0.1, label=prog, color=cmap[prog])

        fig.legend()
        fig.show()
        fig.savefig(f'{time}_{type_}.png')
