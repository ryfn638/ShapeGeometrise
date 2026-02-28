
/*
* Create an existing shape, each shape created this way will be slightly spoofed from an existing shape
* The spoofed shape is typically the shape that performed the best in terms of canvas score.
*/
shape_t createGeneration(shape_t* bestShape, vector<vector<Colour>> canvas)


/*
* Calculate the performance of how a shape fits on to a canvas, typically this doesnt need to be redone this is already performed in the generation
*/
int calculateCanvasScore(vector<vector<Colour>> canvas, shape_t shape)


/*
* Projects a new shape on to the canvas, 
*/
void project_canvas(vector<vector<Colour>> &canvas, shape_t shape)