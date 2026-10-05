#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "./image.hpp"

class Wrapper
{
public:
  using images_t = std::vector<Image *>;

private:
  images_t m_images {};

public:
  /**
   * Dumps all images.
   */
  Wrapper()
  {
    size_t count = 0U;

    const auto domain = Il2cpp::get_domain();
    Il2cpp::thread_attach( domain );

    const auto assemblies = Il2cpp::get_assemblies( domain, &count );

    for ( size_t index = 0U; index < count; ++index )
    {
      auto image = Il2cpp::get_image( assemblies[index] );
      if ( image )
        m_images.emplace_back( static_cast<Image *>( image ) );
    }
  }

  /**
   * Returns a pointer to the desired image if found.
   */
  auto get_image( const char * name ) const -> Image *
  {
    std::string search_name = name;
    
    for ( const auto image : m_images )
    {
      std::string img_name = image->get_name();
      
      // Comparação exata
      if ( img_name == search_name )
        return image;
        
      // Comparação sem .dll
      if (search_name.find(".dll") != std::string::npos) {
          if (img_name == search_name.substr(0, search_name.find(".dll")))
              return image;
      }
      
      if (img_name.find(".dll") != std::string::npos) {
          if (search_name == img_name.substr(0, img_name.find(".dll")))
              return image;
      }
    }

    return nullptr;
  }

  /**
   * Returns all images.
   */
  auto get_images() const -> images_t { return m_images; }

  void dump_images() const
  {
    for ( const auto image : m_images )
    {
      printf("[IL2CPP] Image: %s\n", image->get_name());
    }
  }

  /**
   * Checks if there's an active debugger attached to the thread.
   */
  auto is_debugger_attached() const -> bool { return Il2cpp::is_debugger_attached(); }
};
