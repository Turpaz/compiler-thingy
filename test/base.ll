@.str.nl = constant [2 x i8] c"\0A\00"
@.str.0 = constant [14 x i8] c"Hello, World!\00"
declare i32 @printf(ptr noalias nocapture, ...)
declare void @exit(i32)
define i32 @print(ptr %str)
{
	%1 = call i32 (ptr, ...) @printf(ptr %str)
	ret i32 %1
}
define i32 @println(ptr %str)
{
        %nlp = alloca ptr
        store ptr @.str.nl, ptr %nlp
        %nl = load ptr, ptr %nlp
        %1 = call i32 (ptr, ...) @print(ptr %str)
        %2 = call i32 (ptr, ...) @print(ptr %nl)
        %3 = add i32 %1, %2
        ret i32 %3
}
define dso_local i32 @power(i32 %0, i32 %1) {
  %3 = icmp eq i32 %1, 0
  br i1 %3, label %18, label %4
4: ; preds = %2
  %5 = sdiv i32 %1, 2
  %6 = tail call i32 @power(i32 %0, i32 %5)
  %7 = and i32 %1, 1
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %9, label %11
9: ; preds = %4
  %10 = mul nsw i32 %6, %6
  br label %18
11: ; preds = %4
  %12 = icmp sgt i32 %1, 0
  %13 = mul i32 %6, %6
  br i1 %12, label %14, label %16
14: ; preds = %11
  %15 = mul i32 %13, %0
  br label %18
16: ; preds = %11
  %17 = sdiv i32 %13, %0
  br label %18
18: ; preds = %9, %14, %16, %2
  %19 = phi i32 [ 1, %2 ], [ %10, %9 ], [ %15, %14 ], [ %17, %16 ]
  ret i32 %19
}
define i32 @main(i32 %argc,ptr %argv)
{
%e0p = alloca ptr
store ptr @.str.0, ptr %e0p
%e0 = load ptr, ptr %e0p
%1 = call i32 (ptr) @println(ptr %e0)
%int0 = add i32 0, 200
ret i32 %int0
%int1 = add i32 0, 0
ret i32 %int1
}
