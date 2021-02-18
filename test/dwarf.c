#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libdwarf/libdwarf.h>
#include <libdwarf/dwarf.h>

void print_depth_prefix(int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
}

void print_die_info(Dwarf_Debug debug, Dwarf_Die die, int depth)
{
    Dwarf_Half tag;
    dwarf_tag(die, &tag, NULL);

    char const *tagname;
    dwarf_get_TAG_name(tag, &tagname);
    print_depth_prefix(depth);
    printf("%s\n", tagname);

    Dwarf_Attribute *attrlist;
    Dwarf_Signed size;
    dwarf_attrlist(die, &attrlist, &size, NULL);

    for (int i = 0; i < size; i++)
    {
        Dwarf_Half attr_num;
        Dwarf_Half attr_form_num;
        char const *attr_name;
        char const *attr_form_name;
        dwarf_whatattr(attrlist[i], &attr_num, NULL);
        dwarf_whatform(attrlist[i], &attr_form_num, NULL);
        dwarf_get_AT_name(attr_num, &attr_name);
        dwarf_get_FORM_name(attr_form_num, &attr_form_name);

        print_depth_prefix(depth);
        printf(" %s %s", attr_form_name, attr_name);

        if (attr_form_num == DW_FORM_exprloc)
        {
            Dwarf_Unsigned expr_len;
            Dwarf_Ptr block_ptr;
            dwarf_formexprloc(attrlist[i], &expr_len, &block_ptr, NULL);
            printf("\t0x%x length %llu", block_ptr, expr_len);
        }

        printf("\n");
    }
}

void parse_die(Dwarf_Debug debug, Dwarf_Die die, int depth)
{
    Dwarf_Die current_die = die;
    int res;
    while (1)
    {
        print_die_info(debug, current_die, depth);
        Dwarf_Die child, sib_die;

        res = dwarf_child(current_die, &child, NULL);
        if (res == DW_DLV_OK)
            parse_die(debug, child, depth + 1);
        
        res = dwarf_siblingof(debug, current_die, &sib_die, NULL);
        if (current_die != die)
            dwarf_dealloc(debug, current_die, DW_DLA_DIE);
        if (res == DW_DLV_NO_ENTRY)
            break;
        current_die = sib_die;
    }
}

void read_compile_units(Dwarf_Debug debug)
{
    while (1)
    {
        Dwarf_Unsigned cu_header_length;
        Dwarf_Half version_stamp;
        Dwarf_Off abbrev_offset;
        Dwarf_Half address_size;
        
        int res = dwarf_next_cu_header(debug, &cu_header_length, 
            &version_stamp, &abbrev_offset, &address_size, NULL, NULL);
        if (res == DW_DLV_ERROR || res == DW_DLV_NO_ENTRY)
            return;
        
        Dwarf_Die die;
        dwarf_siblingof(debug, NULL, &die, NULL);
        parse_die(debug, die, 0);
        dwarf_dealloc(debug, die, DW_DLA_DIE);
    }
}

int main(int argc, char const *argv[])
{
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    Dwarf_Debug debug;
    dwarf_init(fd, DW_DLC_READ, NULL, NULL, &debug, NULL);
    read_compile_units(debug);
    dwarf_finish(debug, NULL);
    close(fd);
    return 0;
}
