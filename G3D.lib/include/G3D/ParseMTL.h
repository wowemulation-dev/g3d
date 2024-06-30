/**
 \file G3D/ParseMTL.h

 \maintainer Morgan McGuire, http://graphics.cs.williams.edu

 \created 2011-07-19
 \edited  2011-07-22

 Copyright 2002-2011, Morgan McGuire.
 All rights reserved.
*/
#ifndef G3D_ParseMTL_h
#define G3D_ParseMTL_h

#include "G3D/platform.h"
#include "G3D/Table.h"
#include "G3D/ReferenceCount.h"
#include "G3D/Color3.h"
#include <string>

namespace G3D {

class TextInput;

/** \brief Parses Wavefront material (.mtl) files.

    Parsing creates references to texture files on disk, but does not actually
    load those textures.

    \sa G3D::ParseOBJ, G3D::ArticulatedModel
*/
class ParseMTL {
public:

    /** Loaded from the MTL file */
    class Material : public ReferenceCountedObject {
    public:
        typedef shared_ptr<Material> Ref;
        std::string     name;

        /** Path relative to which filenames should be resolved */
        std::string     basePath;

        /** Ambient color of the material, on the range 0-1 */
        Color3          Ka;
        std::string     map_Ka;

        /** Diffuse color of the material, on the range 0-1 */
        Color3          Kd;
        std::string     map_Kd;

        /** Specular color of the material, on the range 0-1. */
        Color3          Ks;
        std::string     map_Ks;

        /** Shininess of the material, on the range 0-1000. */
        float           Ns;

        /** map_bump/bump field filename*/
        std::string     map_bump;
        float           bumpBias;
        float           bumpGain;

        /** Opacity (alpha) level, on the range 0-1, where 1 = opaque (default) */
        float           d;
        std::string     map_d;

        /** Transparency level, on the range 0-1. Amount of light
            transmitted.  This is questionable and does not seem to be
            widely used.*/
        float           Tr;

        /** 1 - Transmission, as processed by 3DS Max 
            (http://casual-effects.blogspot.com/2012/01/translucency-in-obj-mtl-files.html).  Other specification documents
            say that it is transmission (e.g, http://paulbourke.net/dataformats/mtl/) but I haven't found software 
            that implements it that way. */
        Color3          Tf;

        /** emissive? */
        Color3          Ke;
        std::string     map_Ke;

        /** Illumination model enumeration on the range 0-10. */
        int             illum;

        /** Index of refraction */
        float           Ni;

        /** (non-standard extension), for lightmaps */
        std::string     lightmap;


    private:

        Material() : Ka(1.0f), Kd(1.0f), Ks(-1.0f), Ns(10.0f), bumpBias(0.0f), bumpGain(1.0f), d(1.0f), Tr(0.0f), Tf(1.0f), Ke(0.0f), illum(2), Ni(1.0f) {}

    public:
        /** We default Ks to 0.8f if there is no map_Ks. 
        This is non-standard but matches G3D's lighting model better. 
        The specification default(and what we default to when there is a map_Ks) is 1.0f.
        Note that we raise Ks to the 9th power when loading into an Articulated Model.
        */
        static Ref create() {
            return Ref(new Material());
        }
    };
    
    Table<std::string, shared_ptr<Material> > materialTable;

private:

    /** Process one line of an OBJ file */
    void processCommand(TextInput& ti, const std::string& cmd);

    shared_ptr<Material>        m_currentMaterial;

    /** Paths are interpreted relative to this */
    std::string                 m_basePath;

public:

    ParseMTL();

    /** \param basePath Directory relative to which texture filenames are resolved. If "<AUTO>", the 
     path to the TextInput%'s file is used. */
    void parse(TextInput& ti, const std::string& basePath = "<AUTO>");

};


} // namespace G3D


template <> struct HashTrait<shared_ptr<G3D::ParseMTL::Material> > {
    static size_t hashCode(const shared_ptr<G3D::ParseMTL::Material>& k) { return reinterpret_cast<size_t>(k.get()); }
};


#endif // G3D_ParseMTL_h

