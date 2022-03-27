package com.draico.asvappra.neuralnetworks;

public class OperandType {
    private int currentOperandType;
    private boolean isCreated;
    public static int OPERAND_TYPE_FLOAT32 = 0;
    public static int OPERAND_TYPE_INT32 = 1;
    public static int OPERAND_TYPE_UINT32 = 2;
    public static int OPERAND_TYPE_TENSOR_FLOAT32 = 3;
    public static int OPERAND_TYPE_TENSOR_INT32 = 4;
    public static int OPERAND_TYPE_TENSOR_QUANT8_ASYMM = 5;
    public static int OPERAND_TYPE_BOOL = 6;
    public static int OPERAND_TYPE_TENSOR_QUANT16_SYMM = 7;
    public static int OPERAND_TYPE_TENSOR_FLOAT16 = 8;
    public static int OPERAND_TYPE_TENSOR_BOOL8 = 9;
    public static int OPERAND_TYPE_FLOAT16 = 10;
    public static int OPERAND_TYPE_TENSOR_QUANT8_SYMM_PER_CHANNEL = 11;
    public static int OPERAND_TYPE_TENSOR_QUANT16_ASYMM = 12;
    public static int OPERAND_TYPE_TENSOR_QUANT8_SYMM = 13;
    public static int OPERAND_TYPE_TENSOR_QUANT8_ASYMM_SIGNED = 14;
    public static int OPERAND_TYPE_MODEL = 15;
    public static native OperandType newOperandType(int[] valueDimensions, float scale, int operandType, int zeroPoint);
    public native void delete();
}
