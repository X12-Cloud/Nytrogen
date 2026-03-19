from manim import *

class NytrogenLogo(Scene):
    def construct(self):
        # Load your colored SVG
        logo = SVGMobject("../images/nytrogen_colored.svg").scale(2)
        
        # 1. Animate the paths (the orbits)
        self.play(Create(logo), run_time=3, rate_func=linear)
        self.wait(0.5)
        
        # 2. Make it glow or pulse slightly
        self.play(logo.animate.set_stroke(width=2).scale(1.1))
        self.play(logo.animate.scale(1/1.1))
        
        self.wait(2)
