/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
//! \brief  Interface for the classes that interact with system-dependent frameworks to
//          encode video files. Subclasses are responsible to create a pipeline
//          that will attempt to process/compress a raw image stream and create a video
//          file. This interface describes the most basic functionalities that are
//          expected from these pipelines.
#ifndef _ENCODER_INTERFACE_H_
#define _ENCODER_INTERFACE_H_

#include "Video_Type.h"

//STL include files
#include <iostream>
#include <string>
#include <vector>

class Encoder_Interface
{
public:
    /* some types that are useful to exchange information between the encoder and its
     * pipeline and the higher level ISFrame_t_MM_Converter */
    enum Encoding_Loss_Mask
    {
        LOSSLESS = 0x00,
        COLORSPACE_CONVERSION = 0x20,  //RGB<->YCbCr conversion will be required
        COMPRESSION_LOSS = 0x40,       //lossy compression method
        SUBSAMPLING_LOSS = 0x80,       //loss of chroma information(4:4:4 -> 4:2:2 -> 4:1:1,...) 
        DOWNSAMPLING_LOSS = 0x010000,  //storage with less bits than proposed by original FourCC
                                       //the second byte of the mask is kept free to indicate the
                                       //"amount" of downsampling 
    }; //these losses are subjectively ordered by increasing quality impact   

    struct Encoding_Solution
    {
        std::string name;
        Video_Type::FourCC fourcc;
    };
    
    struct Encoding_Offer
    {
        Encoding_Solution solution;
        Encoding_Loss_Mask loss;
    };

    //! Empty constructor
    Encoder_Interface();

    //! Virtual destructor, NB: default copy constructor and assignment operator are used
    //! the destructor should close the file properly
    virtual ~Encoder_Interface();

    //! change the output file name, this must supports multiple call
    //! until the first call to store_bytes
    //! \param[in] filename the filename (WARNING, IT OVERWRITES!)
    //! \return    false in case of error
    virtual bool set_filename(const std::string &filename) = 0;

    //! start the encoder thread once the encoding method has been successfully
    //! negotiated and initialized
    //! \return   false if the thread could not be started
    //! \pre      set_encoding_method was successfully called
    virtual bool run() = 0;

    //! close the video and prepare the encoder for a new file
    virtual void close() = 0;
    
    //! Return the possibilities offered by the encoder for a given video_type.
    //! Note that this does not completely guarantee successful negotiation, some parameters 
    //! (e.g., width, height) might not be compatible with the encoder capabilities
    //! but the error might not be triggered at that stage.
    //! \param[in] a video_type
    //! \param[in] the number of bit per samples (to help in choosing an optimal method)  
    //! \param[out] a set of possible encoding method (with eventual loss)
    virtual std::vector<Encoder_Interface::Encoding_Offer> get_compatible_encoding_methods(const Video_Type &video_type, unsigned int bps) = 0;

    
    //! select an encoding solution, this must supports multiple call
    //! until the first call to store_bytes
    //! \param[in]   The selected encoding solution
    //! \param[in]   A video_type, although the encoder does not create the
    //!              video_type, it will take ownership and handle the deletion
    //! \return      true if the pipeline was successfully established though
    //!              it does not prove that data will flow through
    //! \pre         The encoding solution was proposed by the encoder during
    //!              get_compatible_encoding_method
    //! \pre         store_file was called successfully
    virtual bool set_encoding_method(const Encoding_Solution &encoding_solution, Video_Type &video_Type) = 0;

    //! return the video type 
    //! \return      the video type passed during the last call to set_encoding_method
    //! \pre         The encoding solution was negotiated
    virtual Video_Type get_video_type() = 0;

    //! Send a buffer to the encoder
    //! \param[in] buffer, the buffer
    //! \param[in] size, the size of buffer
    //! \return    the actual number of bytes read from buffer
    //!            (this could be less than size in case of error)
    //! \pre       buffer and size are valid
    virtual unsigned int store_bytes(const unsigned char *buffer, unsigned int size) = 0;

    //! query for state
    //! \return    true if the bus is in error state
    virtual bool bad() const = 0;
    
    //! Encoder factory method
    //! \param[in]  name of the encoder: dshow, gstreamer or raw
    //! \return     the requested encoder
    static Encoder_Interface *create_encoder(const std::string &encoder_name);
    
    //a functor to sort the Encoding_Offers by increasing loss
    //function that follows
    struct offers_sort_comparator
    {
        bool operator()(const Encoder_Interface::Encoding_Offer &offer1,
                        const Encoder_Interface::Encoding_Offer &offer2)
        {
            if (offer1.loss == offer2.loss)
            {
                return offer1.solution.name < offer2.solution.name;
            }
            else
            {
                return offer1.loss < offer2.loss;
            }
        }
    };
};

#endif // _ENCODER_INTERFACE_H_
