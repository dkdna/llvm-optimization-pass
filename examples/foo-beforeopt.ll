; ModuleID = 'examples/foo-beforeopt.ll'
source_filename = "examples/foo1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @compute(i32 %0, i32 %1) #0 {
  %3 = mul nsw i32 %0, 8
  %4 = add nsw i32 %3, %1
  %5 = sdiv i32 %4, 16
  %6 = add nsw i32 %0, %1
  %7 = add nsw i32 %0, 1
  %8 = add nsw i32 %0, %1
  %9 = mul nsw i32 %5, %8
  %10 = add nsw i32 %6, %9
  %11 = add nsw i32 %0, %4
  %12 = add nsw i32 %10, %11
  %13 = icmp sgt i32 %12, 10
  br i1 %13, label %14, label %16

14:                                               ; preds = %2
  %15 = add nsw i32 %12, 0
  br label %18

16:                                               ; preds = %2
  %17 = add nsw i32 %12, 0
  br label %18

18:                                               ; preds = %16, %14
  %19 = add nsw i32 %0, %4
  %20 = add nsw i32 %19, %1
  ret i32 %20
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
  %1 = call i32 @compute(i32 100, i32 2)
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %1)
  ret i32 0
}

declare dso_local i32 @printf(i8*, ...) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1~18.04.2 "}
