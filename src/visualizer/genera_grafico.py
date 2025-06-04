#   INFO:
#   Necessario installare la libreria matplotlib
#   Recupera i dati dai file .csv presenti in /results/
#
#             *     ,MMM8&&&.            *
#                  MMMM88&&&&&    .
#                 MMMM88&&&&&&&
#     *           MMM88&&&&&&&&         .
#                 MMM88&&&&&&&&
#       '         'MMM88&&&&&&'
#                   'MMM8&&&'      *
#          |\___/|
#          )     (             .              '
#         =\     /=
#           )===(       *
#          /     \
#          |     |
#         /       \
#         \       /
#  _/\_/\_/\__  _/_/\_/\_/\_/\_/\_/\_/\_/\_/\_
#  |  |  |  |( (  |  |  |  |  |  |  |  |  |  |
#  |  |  |  | ) ) |  |  |  |  |  |  |  |  |  |
#  |  |  |  |(_(  |  |  |  |  |  |  |  |  |  |
#  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
#  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |

import os
import glob
import matplotlib.pyplot as plt
from matplotlib.widgets import Button, CheckButtons
import numpy as np

try:
    gruppi = []

    ##############################
    #   Leggo i file CSV        #
    ##############################
    
    
    folder_path = 'results/'
    csv_files = glob.glob(os.path.join(folder_path, '*.csv'))

    for file_path in csv_files:
        titolo = os.path.splitext(os.path.basename(file_path))[0]
        tempi = []
        numero_elementi = []

        with open(file_path, 'r') as file:
            for numero_riga, riga in enumerate(file, start=1):
                riga = riga.strip()
                if not riga:
                    continue
                try:
                    values = riga.split(',')
                    if len(values) >= 2:
                        numero, tempo = map(float, values[:2])
                        numero_elementi.append(numero)
                        tempi.append(tempo)
                except ValueError:
                    print(f"Errore in {file_path} alla riga {numero_riga}: valore non valido -> {riga}")

        if numero_elementi and tempi:
            gruppi.append({
                "titolo": titolo,
                "numero_elementi": numero_elementi,
                "tempi": tempi
            })

    if not gruppi:
        print("Nessun dato valido trovato.")
        exit()

    #########################
    #   Creo il grafico     #
    #########################

    colors = plt.cm.tab20(np.linspace(0, 1, len(gruppi)))

    def plot_all_groups(gruppi, ax, visible_groups, colors):
        global is_log_scale
        ax.clear()

        for i, gruppo in enumerate(gruppi):
            if visible_groups[i]:
                label = gruppo["titolo"] if gruppo["titolo"] else f"Gruppo {i+1}"
                ax.plot(
                    gruppo["numero_elementi"], gruppo["tempi"],
                    marker='o', linestyle='-', label=label,
                    color=colors[i]
                )

        ax.set_title("Grafico")
        ax.set_xlabel("Array Length")
        ax.set_ylabel("Time (µs)")
        ax.grid(True)

        if is_log_scale:
            ax.set_xscale('log')
            ax.set_yscale('log')

        if len(ax.get_legend_handles_labels()[0]) > 0:
            ax.legend()

    # Funzione per cambiare la scala tra scala lineare e logaritmica
    def toggle_scale(event):
        global is_log_scale
        is_log_scale = not is_log_scale
        plot_all_groups(gruppi, ax, visible_groups, colors)
        plt.draw()

    # Funzione per aggiornare la visibilità dei gruppi
    def update_visibility(label):
        index = labels.index(label)
        visible_groups[index] = not visible_groups[index]
        plot_all_groups(gruppi, ax, visible_groups, colors)
        update_checkbutton_colors(visible_groups)
        plt.draw()

    # Funzione per aggiornare i colori nel menu di selezione dei grafici
    def update_checkbutton_colors(visible_groups):
        for i, rect in enumerate(check.rectangles):
            if visible_groups[i]:
                rect.set_facecolor(colors[i])
            else:
                rect.set_facecolor('white')

    # Crea grafico e gli assi
    fig, ax = plt.subplots(figsize=(10, 8))
    plt.subplots_adjust(left=0.3, bottom=0.2)

    labels = [gruppo["titolo"] for gruppo in gruppi]
    visible_groups = [True] * len(gruppi)
    
    is_log_scale = False

    plot_all_groups(gruppi, ax, visible_groups, colors)

    # Pulsante per alternare la scala
    ax_button = plt.axes([0.8, 0.05, 0.1, 0.05])    # [x, y, larghezza, altezza]
    button = Button(ax_button, 'Log Scale')
    button.on_clicked(toggle_scale)

    # Pulsanti di selezione visibilità grafici
    ax_check = plt.axes([0.05, 0.5, 0.17, 0.25])     # [x, y, larghezza, altezza]
    check = CheckButtons(ax_check, labels, visible_groups)
    update_checkbutton_colors(visible_groups)

    for spine in ax_check.spines.values():
        spine.set_visible(False)

    check.on_clicked(update_visibility)

    plt.show()

except FileNotFoundError:
    print("File o cartella non trovati")
except IOError:
    print("Errore durante la lettura dei file")

