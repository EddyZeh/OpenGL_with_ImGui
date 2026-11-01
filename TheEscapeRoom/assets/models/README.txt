Put imported models for The Escape Room here (one folder per model), e.g.

    assets/models/desk/scene.gltf
    assets/models/cabinet/cabinet.obj

This folder is copied next to the executable by the post-build step, so a model is loaded with
the path relative to this folder:

    level.replaceWithModel("desk", "desk/scene.gltf", localTransform);

See README.md ("Replacing placeholders with models") and Level::applyModelReplacements().
Check each asset's licence before distributing the game.
