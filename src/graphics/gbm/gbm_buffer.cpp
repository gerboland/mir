/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by:
 *   Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 *   Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "mir/graphics/gbm/gbm_buffer.h"
#include "mir/compositor/buffer_ipc_package.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdexcept>
#include <mutex>

namespace mc=mir::compositor;
namespace mg=mir::graphics;
namespace mgg=mir::graphics::gbm;
namespace geom=mir::geometry;

namespace
{
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR_ = 0;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR_ = 0;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES_ = 0;

std::mutex ext_mutex;

void ensure_egl_image_extensions()
{
    std::lock_guard<std::mutex> lock(ext_mutex);

    if (eglCreateImageKHR_ != 0 && eglDestroyImageKHR_ != 0 &&
        glEGLImageTargetTexture2DOES_ != 0)
    {
        return;
    }

    std::string ext_string;
    const char* exts = eglQueryString(eglGetCurrentDisplay(), EGL_EXTENSIONS);
    if (exts)
        ext_string = exts;

    /* Mesa in the framebuffer doesn't advertise EGL_KHRimage_pixmap properly */
    //if (ext_string.find("EGL_KHRimage_pixmap") != std::string::npos)
    {
        eglCreateImageKHR_ =
            reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
        eglDestroyImageKHR_ =
            reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    }

    if (!eglCreateImageKHR_ || !eglDestroyImageKHR_)
        throw std::runtime_error("EGL implementation doesn't support EGLImage");
    
    exts = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (exts)
        ext_string = exts;
    else
        ext_string.clear();

    if (ext_string.find("GL_OES_EGL_image") != std::string::npos)
    {
        glEGLImageTargetTexture2DOES_ = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
                eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    }

    if (!glEGLImageTargetTexture2DOES_)
        throw std::runtime_error("GLES2 implementation doesn't support updating a texture from an EGLImage");
}

}

void mgg::GBMBufferObjectDeleter::operator()(gbm_bo* handle) const
{
    if (handle)
        gbm_bo_destroy(handle);
}

geom::PixelFormat mgg::gbm_format_to_mir_format(uint32_t format)
{
    (void)format;
    return geom::PixelFormat::rgba_8888;
}

uint32_t mgg::mir_format_to_gbm_format(geom::PixelFormat format)
{
    (void)format;
    return GBM_BO_FORMAT_ARGB8888;
}


mgg::GBMBuffer::GBMBuffer(
    std::unique_ptr<gbm_bo, mgg::GBMBufferObjectDeleter> handle) 
        : gbm_handle(std::move(handle)), egl_image(EGL_NO_IMAGE_KHR)
{
}

mgg::GBMBuffer::~GBMBuffer()
{
    if (egl_image != EGL_NO_IMAGE_KHR)
        (*eglDestroyImageKHR_)(eglGetCurrentDisplay(), egl_image);
}

geom::Size mgg::GBMBuffer::size() const
{
    return {geom::Width{gbm_bo_get_width(gbm_handle.get())},
            geom::Height{gbm_bo_get_height(gbm_handle.get())}};
}

geom::Stride mgg::GBMBuffer::stride() const
{
    return geom::Stride(gbm_bo_get_stride(gbm_handle.get()));
}

geom::PixelFormat mgg::GBMBuffer::pixel_format() const
{
    return gbm_format_to_mir_format(gbm_bo_get_format(gbm_handle.get()));
}

std::shared_ptr<mc::BufferIPCPackage> mgg::GBMBuffer::get_ipc_package() const
{
    auto temp = std::make_shared<mc::BufferIPCPackage>();
    temp->ipc_data.push_back(gbm_bo_get_handle(gbm_handle.get()).u32);
    return temp;
}

void mgg::GBMBuffer::bind_to_texture()
{
    ensure_egl_image();

    (*glEGLImageTargetTexture2DOES_)(GL_TEXTURE_2D, egl_image);
}

void mgg::GBMBuffer::ensure_egl_image()
{
    static const EGLint image_attrs[] =
    {
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        EGL_NONE
    };

    if (egl_image == EGL_NO_IMAGE_KHR)
    {
        ensure_egl_image_extensions();

        egl_image = (*eglCreateImageKHR_)(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
                                          EGL_NATIVE_PIXMAP_KHR, gbm_handle.get(),
                                          image_attrs);
        if (egl_image == EGL_NO_IMAGE_KHR)
            throw std::runtime_error("Failed to create EGLImage from GBM bo");
    }
}
