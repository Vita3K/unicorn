/* Unicorn Emulator Engine */
/* By Nguyen Anh Quynh <aquynh@gmail.com>, 2015 */

#include "qemu/osdep.h"
#include "cpu.h"
#include "hw/boards.h"
#include "hw/arm/arm.h"
#include "sysemu/cpus.h"
#include "unicorn.h"
#include "unicorn_common.h"
#include "uc_priv.h"

const int ARM_REGS_STORAGE_SIZE = offsetof(CPUARMState, tlb_table);

static void arm_set_pc(struct uc_struct *uc, uint64_t address)
{
    CPUArchState *state = uc->cpu->env_ptr;

    state->pc = address;
    state->regs[15] = address;
}

void arm_release(void* ctx);

void arm_release(void* ctx)
{
    TCGContext *s = (TCGContext *) ctx;
    struct uc_struct* uc = s->uc;
    ARMCPU* cpu = ARM_CPU(uc, uc->cpu);
    CPUArchState *env = &cpu->env;

    g_free(cpu->cpreg_indexes);
    g_free(cpu->cpreg_values);
    g_free(cpu->cpreg_vmstate_indexes);
    g_free(cpu->cpreg_vmstate_values);
    g_free(env->pmsav7.drbar);
    g_free(env->pmsav7.drsr);
    g_free(env->pmsav7.dracr);

    release_common(ctx);
}

void arm_reg_reset(struct uc_struct *uc)
{
    CPUArchState *env = uc->cpu->env_ptr;

    memset(env->regs, 0, sizeof(env->regs));

    env->pc = 0;
}

int arm_reg_read(struct uc_struct *uc, unsigned int *regs, void **vals, int count)
{
    CPUState *mycpu = uc->cpu;
    CPUARMState *state = &ARM_CPU(uc, mycpu)->env;
    int i;

    for (i = 0; i < count; i++) {
        unsigned int regid = regs[i];
        void *value = vals[i];
        if (regid >= UC_ARM_REG_R0 && regid <= UC_ARM_REG_R12) {
            *(int32_t *)value = state->regs[regid - UC_ARM_REG_R0];
        } else if (regid >= UC_ARM_REG_D0 && regid <= UC_ARM_REG_D31) {
            const float64 *d_reg = aa32_vfp_dreg(state, regid - UC_ARM64_REG_D0);
            *(float64 *)value = *d_reg;
        } else {
            switch(regid) {
                case UC_ARM_REG_APSR:
                    *(int32_t *)value = cpsr_read(state) & CPSR_NZCV;
                    break;
                case UC_ARM_REG_CPSR:
                    *(int32_t *)value = cpsr_read(state);
                    break;
                //case UC_ARM_REG_SP:
                case UC_ARM_REG_R13:
                    *(int32_t *)value = state->regs[13];
                    break;
                //case UC_ARM_REG_LR:
                case UC_ARM_REG_R14:
                    *(int32_t *)value = state->regs[14];
                    break;
                //case UC_ARM_REG_PC:
                case UC_ARM_REG_R15:
                    *(int32_t *)value = state->regs[15];
                    break;
                case UC_ARM_REG_C1_C0_2:
                    *(int32_t *)value = state->cp15.cpacr_el1;
                    break;
                case UC_ARM_REG_C13_C0_3:
                    *(int32_t *)value = state->cp15.tpidrro_el[0];
                    break;
                case UC_ARM_REG_FPEXC:
                    *(int32_t *)value = state->vfp.xregs[ARM_VFP_FPEXC];
                    break;
            }
        }
    }

    return 0;
}

int arm_reg_write(struct uc_struct *uc, unsigned int *regs, void* const* vals, int count)
{
    CPUState *mycpu = uc->cpu;
    CPUARMState *state = &ARM_CPU(uc, mycpu)->env;
    int i;

    for (i = 0; i < count; i++) {
        unsigned int regid = regs[i];
        const void *value = vals[i];
        if (regid >= UC_ARM_REG_R0 && regid <= UC_ARM_REG_R12) {
            state->regs[regid - UC_ARM_REG_R0] = *(uint32_t *)value;
        } else if (regid >= UC_ARM_REG_D0 && regid <= UC_ARM_REG_D31) {
            float64 *d_reg = aa32_vfp_dreg(state, regid - UC_ARM64_REG_D0);
            *d_reg = *(float64 *)value;
        } else {
            switch(regid) {
                case UC_ARM_REG_APSR:
                    cpsr_write(state, *(uint32_t *)value, CPSR_NZCV, CPSRWriteRaw);
                    break;
                case UC_ARM_REG_CPSR:
                    cpsr_write(state, *(uint32_t *)value, ~0, CPSRWriteRaw);
                    break;
                //case UC_ARM_REG_SP:
                case UC_ARM_REG_R13:
                    state->regs[13] = *(uint32_t *)value;
                    break;
                //case UC_ARM_REG_LR:
                case UC_ARM_REG_R14:
                    state->regs[14] = *(uint32_t *)value;
                    break;
                //case UC_ARM_REG_PC:
                case UC_ARM_REG_R15:
                    state->pc = (*(uint32_t *)value & ~1);
                    state->thumb = (*(uint32_t *)value & 1);
                    state->uc->thumb = (*(uint32_t *)value & 1);
                    state->regs[15] = (*(uint32_t *)value & ~1);
                    // force to quit execution and flush TB
                    uc->quit_request = true;
                    uc_emu_stop(uc);

                    break;
                case UC_ARM_REG_C1_C0_2:
                    state->cp15.cpacr_el1 = *(int32_t *)value;
                    break;

                case UC_ARM_REG_C13_C0_3:
                    state->cp15.tpidrro_el[0] = *(int32_t *)value;
                    break;
                case UC_ARM_REG_FPEXC:
                    state->vfp.xregs[ARM_VFP_FPEXC] = *(int32_t *)value;
                    break;
            }
        }
    }

    return 0;
}

static bool arm_stop_interrupt(int intno)
{
    switch(intno) {
        default:
            return false;
        case EXCP_UDEF:
        case EXCP_YIELD:
            return true;
    }
}

static uc_err arm_query(struct uc_struct *uc, uc_query_type type, size_t *result)
{
    CPUState *mycpu = uc->cpu;
    CPUARMState *state = &ARM_CPU(uc, mycpu)->env;
    uint32_t mode;

    switch(type) {
        case UC_QUERY_MODE:
            // zero out ARM/THUMB mode
            mode = uc->mode & ~(UC_MODE_ARM | UC_MODE_THUMB);
            // THUMB mode or ARM MOde
            mode += ((state->thumb != 0) ? UC_MODE_THUMB : UC_MODE_ARM);
            *result = mode;
            return UC_ERR_OK;
        default:
            return UC_ERR_ARG;
    }
}

#ifdef TARGET_WORDS_BIGENDIAN
void armeb_uc_init(struct uc_struct* uc)
#else
void arm_uc_init(struct uc_struct* uc)
#endif
{
    register_accel_types(uc);
    arm_cpu_register_types(uc);
    tosa_machine_init_register_types(uc);
    uc->reg_read = arm_reg_read;
    uc->reg_write = arm_reg_write;
    uc->reg_reset = arm_reg_reset;
    uc->set_pc = arm_set_pc;
    uc->stop_interrupt = arm_stop_interrupt;
    uc->release = arm_release;
    uc->query = arm_query;
    uc_common_init(uc);
}
