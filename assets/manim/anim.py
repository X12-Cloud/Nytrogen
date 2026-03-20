from manim import *

class NytrogenLogo(Scene):
    def construct(self):
        # 1. Light Mode Setup
        self.camera.background_color = WHITE
        
        # 2. Load and Pre-process
        logo = SVGMobject("assets/images/nytrogen_colored.svg").scale(2)
        
        # We loop through every part to make sure strokes are visible
        for part in logo:
            # If it's black (000000), make it a clear stroke
            # If it has a color, we keep the color but add a stroke for Create()
            part.set_stroke(width=2) 
            if part.get_fill_color() == BLACK:
                part.set_color(BLACK)

        # --- THE ANIMATION ---

        # 3. DRAW EVERYTHING AT ONCE (Geometry Dash Style Speed)
        self.play(
            Create(logo), 
            run_time=1.5, 
            rate_func=rush_from # Start fast, finish smooth
        )
        
        # 4. Final Polish: Fill the colors in after drawing
        self.play(
            logo.animate.set_fill(opacity=1),
            run_time=0.5
        )

        self.wait(2)

# run: uv run --with manim manim -pql assets/manim/anim.py NytrogenLogo
