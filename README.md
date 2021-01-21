# procAnim

This is the project I developed for my bachelor's thesis. Since then, I used it as the starting point for a hobby game project. The finished thesis can be found in the thesis_cleanup branch.

Abstract:

This thesis describes the development of a system for the dynamic animation of
walking movements for two-dimensional characters in video games. To achieve
this, the character model is created as a skeleton and provided with a texture. The
application then uses the user’s input and the data of the skeleton to generate
new animations for every single step of the character. Animations created this way
allow for movements with a speed that is fitted precisely to the player’s input and
that are also able to adapt to differences in ground height.
In order to do this, the system creates Hermite splines for the hands, feet
and pelvis of the character that the respective body parts follow. These splines
are adjusted to the floor below the player and vary the step distance and shape
depending on the required walking speed. Furthermore, these splines are based on a set of prototype splines that the user may set beforehand. This allows for
customization of the created animations in order to fit them to different characters
and use cases. The thesis shows some examples of animations created with the
presented system to showcase different possible variations of movements and the
adaptation to various situations.