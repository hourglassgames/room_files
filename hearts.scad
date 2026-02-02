// hearts.scad
// Extrudes heart.svg from the same folder.

linear_extrude(height = 5)
    import("heart.svg", dpi = 96);
