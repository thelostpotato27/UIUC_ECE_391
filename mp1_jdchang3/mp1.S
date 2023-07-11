
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
	    movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
        pushl %ebp
        movl %esp, %ebp
        pushl %ebx
        pushl %ecx
        pushl %edx
        pushl (mp1_missile_list)
        jmp update_missiles



		ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
		ret


# ----------------- Functions private to this file -------------------

update_missiles:
        popl %ebx
        movl X(%ebx), %ecx
        addl VX(%ebx), %ecx     # add VX to X, store val in ECX
        movl %ecx, X(%ebx)      # store updated X val

        movl Y(%ebx), %edx
        addl YX(%ebx), %edx     # add VY to Y, store val in ECX
        movl %edx, Y(%ebx)      # store updated Y val

        cmpl %ecx, $5242879     # 5242879 is if every bit of the lower 16 bits was 1 and the top 16 bits represented a 79.
        jb delete_missiles      # this is the highest X value possible, This code assumes unsigned
        cmpl $0, %ecx           # checks if less than 0, though it should never be possible
        jb delete_missiles

        cmpl %edx, $1638399     # 5242879 is if every bit of the lower 16 bits was 1 and the top 16 bits represented a 79.
        jb delete_missiles      # this is the highest X value possible, This code assumes unsigned
        cmpl $0, %edx           # checks if less than 0, though it should never be possible
        jb delete_missiles

        

        pushl %ebp              # calling missile_explode 
        movl %esp, %ebp
        pushl %ebx
        call missile_explode
        movl %ebp, %esp
        popl %ebp


        cmpl 
		ret

delete_missiles:

mp1_ioctl_startgame:
		ret

mp1_ioctl_addmissile:
		ret

mp1_ioctl_movexhairs:
		ret

mp1_ioctl_getstatus:
		ret

mp1_ioctl_endgame:
		ret
