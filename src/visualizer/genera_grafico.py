#   INFO:
#   Necessario installare la libreria matplotlib e numpy
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
    gruppi_n = []
    gruppi_m = []
    current_gruppi = None
    interpolate = False
    x_label = "Array Length (n)"

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
            gruppo = {
                "titolo": titolo.replace(".array_length", "").replace(".input_range", ""),
                "numero_elementi": numero_elementi,
                "tempi": tempi
            }
            if ".array_length" in titolo:
                gruppi_n.append(gruppo)
            elif ".input_range" in titolo:
                gruppi_m.append(gruppo)
                
    titles_order = [g["titolo"] for g in gruppi_n]
    current_gruppi = gruppi_n
    
    if not current_gruppi:
        print("Nessun dato valido trovato.")
        exit()

    #########################
    #   Creo il grafico     #
    #########################

    cmap = plt.cm.tab20(np.linspace(0,1,len(titles_order)))
    color_map = { title: cmap[i] for i, title in enumerate(titles_order) }

    def plot_all_groups(current_gruppi, ax, visible_groups):
        global is_log_scale
        ax.clear()

        for title in titles_order:
                for idx, g in enumerate(current_gruppi):
                    if g["titolo"] == title and visible_groups[idx]:
                        x = g["numero_elementi"]
                        y = g["tempi"]
                        
                        if interpolate:
                            ax.plot(x, y, 'o', color=color_map[title], markersize=3, label=title)
                            # Interpolazione con polinomio di grado 5
                            z = np.polyfit(x, y, deg=5)
                            p = np.poly1d(z)
                            x_interp = np.linspace(min(x), max(x), 300) # 300 punti tra punto max e min
                            ax.plot(x_interp, p(x_interp), '-', color=color_map[title])
                        else:
                            ax.plot(x, y, marker='o', linestyle='-', color=color_map[title], label=title, markersize=3)

                        break

        ax.set_title("Comparison of Sorting Algorithms")
        ax.set_xlabel(x_label)
        ax.set_ylabel("Time (s)")
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
        
        if is_log_scale:
            btn_log.label.set_text('Linear Scale')
        else:
            btn_log.label.set_text('Log Scale')
            
        fig.canvas.draw_idle()
        
        plot_all_groups(current_gruppi, ax, visible_groups)
        plt.draw()

    # Funzione per aggiornare la visibilità dei gruppi
    def update_visibility(label):
        index = labels.index(label)
        visible_groups[index] = not visible_groups[index]
        plot_all_groups(current_gruppi, ax, visible_groups)
        update_checkbutton_colors(visible_groups)
        plt.draw()

    # Funzione per aggiornare i colori nel menu di selezione dei grafici
    def update_checkbutton_colors(visible_groups):
        for i, rect in enumerate(check.rectangles):
           title = labels[i]
           rect.set_facecolor(color_map[title] if visible_groups[i] else 'white')

    # Crea grafico e gli assi
    fig, ax = plt.subplots(figsize=(10, 8))
    plt.subplots_adjust(left=0.3, bottom=0.2)

    labels = [gruppo["titolo"] for gruppo in current_gruppi]
    visible_groups = [True] * len(current_gruppi)
    
    is_log_scale = False

    plot_all_groups(current_gruppi, ax, visible_groups)

    # Pulsante per alternare la scala
    ax_button = plt.axes([0.8, 0.05, 0.1, 0.05])    # [x, y, larghezza, altezza]
    btn_log = Button(ax_button, 'Log Scale')
    btn_log.on_clicked(toggle_scale)
    
    
    # Pulsante per alternare il dataset tra n e m
    ax_button_mode = plt.axes([0.68, 0.05, 0.1, 0.05])
    button_mode = Button(ax_button_mode, 'Input Range')

    def toggle_dataset(event):
        global current_gruppi, visible_groups, labels, check, x_label

        if current_gruppi is gruppi_n:
            current_gruppi = gruppi_m
            button_mode.label.set_text("Array Length")
            x_label = "Input Range (m)"
            ax.set_xlabel(x_label)
        else:
            current_gruppi = gruppi_n
            button_mode.label.set_text("Input Range")
            x_label = "Array Length (n)" 
            ax.set_xlabel(x_label)

        labels = [gruppo["titolo"] for gruppo in current_gruppi]
        visible_groups[:] = [True] * len(current_gruppi)
                
                
        for child in fig.axes:
            if getattr(child, '_is_check', False) and child != ax_interp:
                fig.delaxes(child)

        ax_check = plt.axes([0.05, 0.5, 0.17, 0.25])
        ax_check._is_check = True
        check = CheckButtons(ax_check, labels, visible_groups)
        check.on_clicked(update_visibility)
        for spine in ax_check.spines.values():
            spine.set_visible(False)
        update_checkbutton_colors(visible_groups)

        plot_all_groups(current_gruppi, ax, visible_groups)
        fig.canvas.draw_idle()
        plt.pause(0.01)

    button_mode.on_clicked(toggle_dataset)
    
    
    # Pulsante per interpolazione
    ax_interp = plt.axes([0.56, 0.05, 0.11, 0.05])
    ax_interp._is_check = True
    check_interp = CheckButtons(ax_interp, ["Interpolate"], [interpolate])
    for spine in ax_interp.spines.values():
        spine.set_visible(False)

    def toggle_interpolation(label):
        global interpolate
        interpolate = not interpolate
        plot_all_groups(current_gruppi, ax, visible_groups)
        update_checkbutton_colors(visible_groups)
        plt.draw()

    check_interp.on_clicked(toggle_interpolation)

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

