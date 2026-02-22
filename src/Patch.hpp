#ifndef PATCH_HPP_
#define PATCH_HPP_

#include <cstdint>
#include <cstdio>
#include <lv2/atom/atom.h>
#include <lv2/atom/forge.h>
#include <lv2/patch/patch.h>
#include <lv2/urid/urid.h>

/**
Partial C++ implementation to read LV2 patches. All methods apply on other
classes and do not change the object state. The implementation is assumed
to be realtime safe after construction.

For the creation of LV2 patch atoms use class PatchFactory.
 */
class Patch
{
public:
    // LV2_URID atom_Blank LV2_DEPRECATED;
    LV2_URID atom_Bool;
    LV2_URID atom_Chunk;
    LV2_URID atom_Double;
    LV2_URID atom_Float;
    LV2_URID atom_Int;
    LV2_URID atom_Long;
    LV2_URID atom_Literal;
    LV2_URID atom_Object;
    LV2_URID atom_Path;
    LV2_URID atom_Property;
    // LV2_URID atom_Resource LV2_DEPRECATED;
    LV2_URID atom_Sequence;
    LV2_URID atom_String;
    LV2_URID atom_Tuple;
    LV2_URID atom_URI;
    LV2_URID atom_URID;
    LV2_URID atom_Vector;
    LV2_URID patch_Set;
    LV2_URID patch_Get;
    LV2_URID patch_subject;
    LV2_URID patch_property;
    LV2_URID patch_value;

    inline Patch() {}

    /**
    Creates a Patch object with an empty atom buffer.
    @param map      URID map object.
     */
    inline Patch(LV2_URID_Map* map)
    {
        if (!map) 
        {
            fprintf(stderr, "Can't init Patch with NULL.\n");
            return;
        }

        // atom_Blank    = map->map(map->handle, LV2_ATOM__Blank);
        atom_Bool     = map->map(map->handle, LV2_ATOM__Bool);
        atom_Chunk    = map->map(map->handle, LV2_ATOM__Chunk);
        atom_Double   = map->map(map->handle, LV2_ATOM__Double);
        atom_Float    = map->map(map->handle, LV2_ATOM__Float);
        atom_Int      = map->map(map->handle, LV2_ATOM__Int);
        atom_Long     = map->map(map->handle, LV2_ATOM__Long);
        atom_Literal  = map->map(map->handle, LV2_ATOM__Literal);
        atom_Object   = map->map(map->handle, LV2_ATOM__Object);
        atom_Path     = map->map(map->handle, LV2_ATOM__Path);
        atom_Property = map->map(map->handle, LV2_ATOM__Property);
        // atom_Resource = map->map(map->handle, LV2_ATOM__Resource);
        atom_Sequence = map->map(map->handle, LV2_ATOM__Sequence);
        atom_String   = map->map(map->handle, LV2_ATOM__String);
        atom_Tuple    = map->map(map->handle, LV2_ATOM__Tuple);
        atom_URI      = map->map(map->handle, LV2_ATOM__URI);
        atom_URID     = map->map(map->handle, LV2_ATOM__URID);
        atom_Vector   = map->map(map->handle, LV2_ATOM__Vector);
        patch_Set     = map->map(map->handle, LV2_PATCH__Set);
        patch_Get     = map->map(map->handle, LV2_PATCH__Get);
        patch_subject = map->map(map->handle, LV2_PATCH__subject);
        patch_property= map->map(map->handle, LV2_PATCH__property);
        patch_value   = map->map(map->handle, LV2_PATCH__value);
    }

    /**
    Checks if an atom is a LV2 patch message.
    @param atom     Pointer to LV2 atom.
    @return         True, if atom is an LV2 patch message.

    @todo           Only patch_Get and patch_Set supported yet. Extend!
     */
    inline bool is_Patch_Msg(const LV2_Atom* atom) const
    {
        if ((!atom) || (atom->type != atom_Object)) return false;
        const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);
        return ((obj->body.otype == patch_Get) || (obj->body.otype == patch_Set));
    } 

    /**
    Gets the Patch message type URID from an atom.
    @param atom     Pointer to an LV2 atom.
    @return         URID of the Patch message type of the atom. Returns 0 if
                    the atom is not a valid LV2 patch message. Thus, returning 
                    0 needs to be further validated.
     */
    inline LV2_URID get_message_type(const LV2_Atom* atom) const
    {
        if ((!atom) || (atom->type != atom_Object)) return 0;
        const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);
        return obj->body.otype;
    }

    /**
    Gets the Patch subject URID from an atom (if provided).
    @param atom     Pointer to an LV2 atom.
    @return         URID of the Patch subject of the atom. Returns 0 if
                    the atom is not a valid LV2 patch message. Thus, returning 
                    0 needs to be further validated.

    @todo           Only patch_Get and patch_Set supported yet. Extend!
     */
    inline LV2_URID get_subject_type(const LV2_Atom* atom) const
    {
        if ((!atom) || (atom->type != atom_Object)) return 0;
        const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);
        if ((obj->body.otype != patch_Get) && (obj->body.otype != patch_Set)) return 0;
        const LV2_Atom* subject = nullptr;
        lv2_atom_object_get (obj, patch_subject, &subject, nullptr);
        if ((!subject) || (subject->type != atom_URID)) return 0;
        const LV2_Atom_URID* urid = reinterpret_cast<const LV2_Atom_URID*>(subject);
        return urid->body;
    }

    /**
    Gets the Patch propery URID from an atom.
    @param atom     Pointer to an LV2 atom.
    @return         URID of the Patch property of the atom. Returns 0 if
                    the atom is not a valid LV2 patch message. Thus, returning 
                    0 needs to be further validated.

    @todo           Only patch_Get and patch_Set supported yet. Extend!
     */
    inline LV2_URID get_property_type(const LV2_Atom* atom) const
    {
        if ((!atom) || (atom->type != atom_Object)) return 0;
        const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);
        if ((obj->body.otype != patch_Get) && (obj->body.otype != patch_Set)) return 0;
        const LV2_Atom* property = nullptr;
        lv2_atom_object_get (obj, patch_property, &property, nullptr);
        if ((!property) || (property->type != atom_URID)) return 0;
        const LV2_Atom_URID* urid = reinterpret_cast<const LV2_Atom_URID*>(property);
        return urid->body;
    }

    /**
    Gets the LV2 value atom from a Patch.
    @param atom     Pointer to an LV2 atom.
    @return         Pointer to an LV2_Atom containg the value.

    @todo           Only patch_Get and patch_Set supported yet. Extend!
     */
    inline const LV2_Atom* get_value_atom(const LV2_Atom* atom) const
    {
        if ((!atom) || (atom->type != atom_Object)) return nullptr;
        const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);
        if ((obj->body.otype != patch_Get) && (obj->body.otype != patch_Set)) return nullptr;
        const LV2_Atom* value = nullptr;
        lv2_atom_object_get (obj, patch_value, &value, nullptr);
        return value;
    }

    /**
    Gets the raw data from a Patch.
    @param atom     Pointer to an LV2 atom.
    @return         Pointer to the raw data stored in the value property.

    @todo           Only patch_Get and patch_Set supported yet. Extend!
     */
    inline const void* get_value_raw(const LV2_Atom* atom) const
    {
        if ((!atom) || (atom->type != atom_Object)) return nullptr;
        const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);
        if ((obj->body.otype != patch_Get) && (obj->body.otype != patch_Set)) return nullptr;
        const LV2_Atom* value = nullptr;
        lv2_atom_object_get (obj, patch_value, &value, nullptr);
        if (!value) return nullptr;
        return value + 1;
    }

    /**
    Gets long data from a Patch.
    @param atom     Pointer to an LV2 atom.
    @return         Long data stored in the value property. Returns 0 for any
                    case of invalid data. Thus, returning 0 needs to be 
                    further validated.

    @todo           Only patch_Get and patch_Set supported yet. Extend!
     */
    inline const uint64_t get_value_long(const LV2_Atom* atom) const
    {
        if ((!atom) || (atom->type != atom_Object)) return 0;
        const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);
        if ((obj->body.otype != patch_Get) && (obj->body.otype != patch_Set)) return 0;
        const LV2_Atom* value = nullptr;
        lv2_atom_object_get (obj, patch_value, &value, nullptr);
        if (!value) return 0;
        return reinterpret_cast<const LV2_Atom_Long*>(value)->body;
    }

    /**
    Gets int data from a Patch.
    @param atom     Pointer to an LV2 atom.
    @return         Int data stored in the value property. Returns 0 for any
                    case of invalid data. Thus, returning 0 needs to be 
                    further validated.

    @todo           Only patch_Get and patch_Set supported yet. Extend!
     */
    inline const int32_t get_value_int(const LV2_Atom* atom) const
    {
        if ((!atom) || (atom->type != atom_Object)) return 0;
        const LV2_Atom_Object* obj = reinterpret_cast<const LV2_Atom_Object*>(atom);
        if ((obj->body.otype != patch_Get) && (obj->body.otype != patch_Set)) return 0;
        const LV2_Atom* value = nullptr;
        lv2_atom_object_get (obj, patch_value, &value, nullptr);
        if (!value) return 0;
        return reinterpret_cast<const LV2_Atom_Int*>(value)->body;
    }

    /**
    Writes a Patch:Get request object into a provided forge buffer.
    @param forge    Forge buffer (by reference).
    @param subject  Patch subject.
    @param property Patch property.
    @return         Reference to the created Patch:Get object atom, or 0 if 
                    failed.
    */
    inline LV2_Atom_Forge_Ref write_patch_Get(LV2_Atom_Forge& forge, const LV2_URID subject, const LV2_URID property)
    {
        LV2_Atom_Forge_Frame frame;
        LV2_Atom_Forge_Ref msg = lv2_atom_forge_object(&forge, &frame, 0, patch_Get);
        msg =  (msg &&
                lv2_atom_forge_key(&forge, patch_subject) &&
                lv2_atom_forge_urid(&forge, subject) &&
                lv2_atom_forge_key(&forge, patch_property) &&
                lv2_atom_forge_urid(&forge, property)) ? msg : 0;
        lv2_atom_forge_pop(&forge, &frame);
        return msg;
    }

    /**
    Writes a Patch:Set Int object into a provided forge buffer.
    @param forge    Forge buffer (by reference).
    @param subject  Patch subject.
    @param property Patch property.
    @param value    Int value
    @return         Reference to the created Patch:Set object atom, or 0 if 
                    failed.
    */
    inline LV2_Atom_Forge_Ref write_patch_Set_Int(LV2_Atom_Forge& forge, const LV2_URID subject, const LV2_URID property, const int32_t value)
    {
        LV2_Atom_Forge_Frame frame;
        LV2_Atom_Forge_Ref msg = lv2_atom_forge_object(&forge, &frame, 0, patch_Set);
        msg =  (msg &&
                lv2_atom_forge_key(&forge, patch_subject) &&
                lv2_atom_forge_urid(&forge, subject) &&
                lv2_atom_forge_key(&forge, patch_property) &&
                lv2_atom_forge_urid(&forge, property) &&
                lv2_atom_forge_key(&forge, patch_value) &&
		        lv2_atom_forge_int(&forge, value)) ? msg : 0;
        lv2_atom_forge_pop(&forge, &frame);
        return msg;
    }

    /**
    Writes a Patch:Set Long object into a provided forge buffer.
    @param forge    Forge buffer (by reference).
    @param subject  Patch subject.
    @param property Patch property.
    @param value    Long value
    @return         Reference to the created Patch:Set object atom, or 0 if 
                    failed.
    */
    inline LV2_Atom_Forge_Ref write_patch_Set_Long(LV2_Atom_Forge& forge, const LV2_URID subject, const LV2_URID property, const int64_t value)
    {
        LV2_Atom_Forge_Frame frame;
        LV2_Atom_Forge_Ref msg = lv2_atom_forge_object(&forge, &frame, 0, patch_Set);
        msg =  (msg &&
                lv2_atom_forge_key(&forge, patch_subject) &&
                lv2_atom_forge_urid(&forge, subject) &&
                lv2_atom_forge_key(&forge, patch_property) &&
                lv2_atom_forge_urid(&forge, property) &&
                lv2_atom_forge_key(&forge, patch_value) &&
		        lv2_atom_forge_long(&forge, value)) ? msg : 0;
        lv2_atom_forge_pop(&forge, &frame);
        return msg;
    }

    /**
    Writes a Patch:Set Double object into a provided forge buffer.
    @param forge    Forge buffer (by reference).
    @param subject  Patch subject.
    @param property Patch property.
    @param value    Double value
    @return         Reference to the created Patch:Set object atom, or 0 if 
                    failed.
    */
    inline LV2_Atom_Forge_Ref write_patch_Set_Double(LV2_Atom_Forge& forge, const LV2_URID subject, const LV2_URID property, const double value)
    {
        LV2_Atom_Forge_Frame frame;
        LV2_Atom_Forge_Ref msg = lv2_atom_forge_object(&forge, &frame, 0, patch_Set);
        msg =  (msg &&
                lv2_atom_forge_key(&forge, patch_subject) &&
                lv2_atom_forge_urid(&forge, subject) &&
                lv2_atom_forge_key(&forge, patch_property) &&
                lv2_atom_forge_urid(&forge, property) &&
                lv2_atom_forge_key(&forge, patch_value) &&
		        lv2_atom_forge_double(&forge, value)) ? msg : 0;
        lv2_atom_forge_pop(&forge, &frame);
        return msg;
    }

    /**
    Writes a Patch:Set Path object into a provided forge buffer.
    @param forge    Forge buffer (by reference).
    @param subject  Patch subject.
    @param property Patch property.
    @param len      String length of path.
    @param path     C string. `path` need not be NULL terminated.
    @return         Reference to the created Patch:Set object atom, or 0 if 
                    failed.
    */
    inline LV2_Atom_Forge_Ref write_patch_Set_Path(LV2_Atom_Forge& forge, const LV2_URID subject, const LV2_URID property, const uint32_t len, const char* path)
    {
        LV2_Atom_Forge_Frame frame;
        LV2_Atom_Forge_Ref msg = lv2_atom_forge_object(&forge, &frame, 0, patch_Set);
        msg =  (msg &&
                lv2_atom_forge_key(&forge, patch_subject) &&
                lv2_atom_forge_urid(&forge, subject) &&
                lv2_atom_forge_key(&forge, patch_property) &&
                lv2_atom_forge_urid(&forge, property) &&
                lv2_atom_forge_key(&forge, patch_value) &&
		        lv2_atom_forge_path(&forge, path, len)) ? msg : 0;
        lv2_atom_forge_pop(&forge, &frame);
        return msg;
    }

    /**
    Writes a Patch:Set Vector object into a provided forge buffer.
    @param forge        Forge buffer (by reference).
    @param subject      Patch subject.
    @param property     Patch property.
    @param child_size   Size of each child in bytes.
    @param Child_type   Type of each child.
    @param n_elems      Number of elements.
    @param data         Pointer to data array.
    @return         Reference to the created Patch:Set object atom, or 0 if 
                    failed.
    */
    inline LV2_Atom_Forge_Ref write_patch_Set_Vector(LV2_Atom_Forge& forge, 
                                                     const LV2_URID subject, 
                                                     const LV2_URID property,
                                                     const uint32_t child_size,
                                                     const LV2_URID child_type,
                                                     const uint32_t n_elems,
                                                     const void* data)
    {
        LV2_Atom_Forge_Frame frame;
        LV2_Atom_Forge_Ref msg = lv2_atom_forge_object(&forge, &frame, 0, patch_Set);
        msg =  (msg &&
                lv2_atom_forge_key(&forge, patch_subject) &&
                lv2_atom_forge_urid(&forge, subject) &&
                lv2_atom_forge_key(&forge, patch_property) &&
                lv2_atom_forge_urid(&forge, property) &&
                lv2_atom_forge_key(&forge, patch_value) &&
		        lv2_atom_forge_vector(&forge, child_size, child_type, n_elems, data)) ? msg : 0;
        lv2_atom_forge_pop(&forge, &frame);
        return msg;
    }
};

#endif /*PATCH_HPP_*/