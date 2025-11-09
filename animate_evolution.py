import matplotlib.pyplot as plt
import matplotlib.animation as animation
import json, glob

fig, ax = plt.subplots()
frames = []

for snap in sorted(glob.glob("PSP-UAV_01_a_gen*.json")):
    data = json.load(open(snap))
    grid = data["heatmap"]  # o similar según dump_json
    im = plt.imshow(grid, cmap="hot", animated=True)
    frames.append([im])

ani = animation.ArtistAnimation(fig, frames, interval=800, blit=True)
ani.save("evolucion_PSP_UAV_01_a.gif", writer="pillow")
plt.show()