Also known as "Really Bad Technical Documentation"

These are mostly notes I create as I go along, marking potential gotchas for people who might try to extend my code.
Some day I'll try and add an overview of the code structure, what each class does, etc.

Development Environment
=======================

I personally launch ES in windowed mode with a smaller resolution than my monitor and with debug text enabled.

`emulationstation --windowed --debug -w 1280 -h 720`


Creating a new GuiComponent
===========================

You probably want to override:

	`bool input(InputConfig* config, Input input);`
		Check if some input is mapped to some action with `config->isMappedTo("a", input);`.
		Check if an input is "pressed" with `input.value != 0` (input.value *can* be negative in the case of axes).

	`void update(int deltaTime);`
		`deltaTime` is in milliseconds.

	`void render(const Eigen::Affine3f& parentTrans);`
		You probably want to do `Eigen::Affine3f trans = parentTrans * getTransform();` to get your final "modelview" matrix.
		Apply the modelview matrix with `Renderer::setMatrix(const Eigen::Affine3f&)`.
		Render any children the component may have with `renderChildren(parentTrans);`.


Creating a new GameListView Class
=================================

1. Don't allow the user to navigate to the root node's parent. If you use a stack of some sort to keep track of past cursor states this will be a natural side effect.
