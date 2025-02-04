#include "/home/justin/install/Halide/build/include/Halide.h"
#include <stdio.h>


#define KERNEL_WIDTH 3
#define KERNEL_HEIGHT 3
#define SCENCE_WIDTH 256
#define SCENCE_HEIGHT 256


int main() {
    
    static constexpr uint32_t padded_width = SCENCE_WIDTH + KERNEL_WIDTH - 1; 
    static constexpr uint32_t padded_height = SCENCE_HEIGHT + KERNEL_HEIGHT - 1; 

    Halide::Buffer<uint8_t> input(padded_width, padded_height); 
    Halide::Var x, y, c;
    
    Halide::Func upcast;
    upcast(x, y) = Halide::cast<uint16_t>(input(x, y));

    Halide::Func blur_x;
    blur_x(x, y) = (input(x-1, y) + 2*input(x, y) + input(x+1, y)) / 4;
 
    Halide::Func blur_y;
    blur_y(x, y) = (blur_x(x, y-1) + 2*blur_x(x, y) + blur_x(x, y-1)) / 4;
    
    Halide::Func downcast;
    downcast(x, y) = Halide::cast<uint8_t>(blur_y(x, y));
    
    Halide::Buffer<uint8_t> result(SCENCE_WIDTH, SCENCE_HEIGHT);
    result.set_min(KERNEL_WIDTH / 2, KERNEL_HEIGHT/2);
    
    downcast.realize(result);

    // Halide::clamp();

    return 0;
}

// void init(Halide::Buffer<uint8_t>& board) { 
//     for (int y = 0; y < board.height(); y++) {
//         for (int x = 0; x < board.width(); x++) {
//             board(x, y) = static_cast<uint8_t>((1 + x + 3 * y) % 2);
//         }
//     }
// }


// int gol(int argc, char **argv) {
    
        
 //    //// Create the board, which is a buffer of uint8 booleans
 //    int width = 5;
 //    int height = 5;
 //    Halide::Buffer<uint8_t> board(width, height); 
 //    init(board);
 //
 //    /// Create a function which lets us get the value at a coorindate.
 //    Halide::Var x, y;
 //    
 //    // const auto getCell = [&](int x, int y) {
 //    //     auto xc = Halide::clamp(x, 0, width);
 //    //     auto yc = Halide::clamp(y, 0, height);
 //    //     return xc + yc;
 //    // };
 //
 //    Halide::Func update; 
 //    update(x, y) = board(
 //        Halide::clamp(x-1, 0, width),
 //        Halide::clamp(y-1, 0, height)
 //    ) * 2;
 //    
 //    Halide::RDom r(0, 10);
 //    Halide::Buffer<uint8_t> output = update.realize({width, height});
 // 
 //    for (int j = 0; j < output.height(); j++) {
 //        for (int i = 0; i < output.width(); i++) {
 //            printf("%d ", output(i, j)); 
 //        }
 //        printf("\n");
 //    }
 //    printf("Success!\n");
 //
 //
    // Halide::Buffer<int32_t> input(10); 
    // Halide::Func addOne("addOne");
    // Halide::Var x;
    // addOne(x) = x + 1;
    //
    // Halide::Buffer<int32_t> output = addOne.realize({10});
    // addOne.compile_to_lowered_stmt("gradient.stmt.html", {}, Halide::HTML);
    
    // Halide::load_image();
//     return 0;
// }
