/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

//! \brief  A class which directly stores the incoming stream into tiff files.

#ifndef _TIFF_ENCODER_H_
#define _TIFF_ENCODER_H_

//the base interface
#include "Encoder_Interface.h"
#include "Tiff_Common.h"


//STL include files
#include <fstream>
#include <vector>

class Tiff_Encoder: public Encoder_Interface
{
public:
    //! Constructor, NB: default copy constructor and assignment operator are used
    Tiff_Encoder();

    //! Virtual destructor, the destructor close the fstream if !done
    ~Tiff_Encoder();

    //! Initialize the object with an output video file and try to start
    //! encoding (this is the base filename, each successive tiff is indexed)
    //! \param[in] filename the filename (WARNING, IT OVERWRITES!)
    //! \return    true if the file can be stored properly
    //! \pre       negotiation done 
    bool set_filename(const std::string &filename) override;

    //! Open file for writing
    //! \param[in] filename the filename (WARNING, IT OVERWRITES!)
    //! \return    true if the file can be stored properly
    //! \pre       negotiation done
    bool open_filename(const std::string &filename);

    //! Write tiff header in the currently opened file
    //! \pre       _file_stream open and ready for writing
    void write_tiff_header();

    //! Return the possibilities offered by the encoder for a given video_type (its fourcc).
    //! \param[in] a video_type
    //! \param[in] the number of bit per samples (to help in choosing an optimal method)  
    //! \param[out] a set of possible encoding method
    std::vector<Encoder_Interface::Encoding_Offer> get_compatible_encoding_methods(const Video_Type &video_type, unsigned int bps) override;
    
    //! Set up the encoder
    //! \param[in]   The selected encoding solution
    //! \param[in]   A video_type, although the encoder does not create the
    //!              video_type, it will take ownership and handle the deletion
    //!              note that the video type is modified to match the encoding
    //!              system properly (eg, little endian/big endian, tiff are inversed, ...)
    //! \return      true if the pipeline was successfully established though
    //!              it does not prove that data will flow through
    //! \pre         The encoding solution was proposed by the encoder during
    //!              get_compatible_encoding_method
    //! \pre         store_file was called successfully
    bool set_encoding_method(const Encoding_Solution &encoding_solution, Video_Type &video_Type) override;

    //! Run is without effect in Tiff_Encoder since there is no encoder thread
    //! and all the write requests are done in the caller context
    //! \return    always true
    bool run() override
    {
        return true;
    };

    //! Close the video and prepare the decoder for a new file
    void close() override;
    
    //! Send a buffer to the encoder
    //! \param[in] buffer, the buffer
    //! \param[in] size, the size of buffer
    //! \return    the actual number of bytes read from buffer
    //!            (this could be less than size in case of error)
    //! \pre       negotiation is over, buffer and size are valid
    unsigned int store_bytes(const unsigned char * buffer, unsigned int size) override;

    //! Query for state
    //! \return    true if the bus is in error state
    bool bad() const override;

    //! Return the video type
    //! \return      the video type passed during the last call to set_encoding_method
    //! \pre         The encoding solution was negotiated
    Video_Type get_video_type() override;
    
private:
    Tiff_Common::t_tiff_info tiff_out_info;
    
    // The output fstream
    std::ofstream _file_stream;
    std::string endian;
    unsigned int counter = 0;    

    // The filename
    std::string basename;
    std::string extension;

    // The associated Video_Type object
    Video_Type _video_type;
    const int FILE_TYPE_TIFF_LE = 0x4949; // little-endian TIFF format
    const int FILE_TYPE_TIFF_BE = 0x4D4D; // big-endian TIFF format    

    // Storage space: for each fourcc there is a corresponding lossless offer.
    // nevertheless, a unique name is used for each "Image_Format".
    std::vector<Encoding_Offer> offers;
    void initialise_variables();

    // Keep track of the current frame and status
    int frame_id = 0;
    unsigned int bytes_count;
    unsigned int bytes_per_frame;
};
#endif // _TIFF_ENCODER_H_