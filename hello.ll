@n = global i32 0

declare void @putch(i32)

declare void @putint(i32)

declare void @starttime()

declare void @stoptime()

declare i32 @getch()

declare i32 @getint()

declare i32 @getarray(i32*)

declare void @putarray(i32, i32*)

define i32 @insertsort(i32* %p_0) {
$entry:
  %temp_2 = alloca i32
  %bbbbbb9_2 = alloca i32
  %j_2 = alloca i32
  %a_1 = alloca i32*
  store i32* %p_0, i32** %a_1
  %i_1 = alloca i32
  store i32 1, i32* %i_1
  br label %$while_entry0

$while_entry0:
  %$0 = load i32, i32* %i_1
  %$1 = load i32, i32* @n
  %$2 = icmp slt i32 %$0, %$1
  %$3 = zext i1 %$2 to i32
  %$4 = icmp ne i32 %$3, 0
  br i1 %$4, label %$while_body0, label %$while_end0

$while_body0:
  %$5 = load i32, i32* %i_1
  %$6 = load i32*, i32** %a_1
  %$ptr0 = getelementptr inbounds i32, i32* %$6, i32 %$5
  %$7 = load i32, i32* %$ptr0
  store i32 %$7, i32* %temp_2
  %$8 = load i32, i32* %i_1
  %$9 = sub i32 %$8, 1
  store i32 %$9, i32* %j_2
  br label %$while_entry1

$while_end0:
  ret i32 0

$while_entry1:
  store i32 1, i32* %bbbbbb9_2
  %$10 = load i32, i32* %j_2
  %$11 = sub i32 0, 1
  %$12 = icmp sgt i32 %$10, %$11
  %$13 = zext i1 %$12 to i32
  %$14 = icmp eq i32 0, %$13
  %$15 = zext i1 %$14 to i32
  %$16 = icmp ne i32 %$15, 0
  br i1 %$16, label %$scthen0, label %$scelse0

$scthen0:
  store i32 0, i32* %bbbbbb9_2
  br label %$scend0

$scelse0:
  %$17 = load i32, i32* %temp_2
  %$18 = load i32, i32* %j_2
  %$19 = load i32*, i32** %a_1
  %$ptr1 = getelementptr inbounds i32, i32* %$19, i32 %$18
  %$20 = load i32, i32* %$ptr1
  %$21 = icmp slt i32 %$17, %$20
  %$22 = zext i1 %$21 to i32
  %$23 = icmp eq i32 0, %$22
  %$24 = zext i1 %$23 to i32
  %$25 = icmp ne i32 %$24, 0
  br i1 %$25, label %$scthen0, label %$scend0

$scend0:
  %$26 = load i32, i32* %bbbbbb9_2
  %$27 = icmp ne i32 %$26, 0
  br i1 %$27, label %$while_body1, label %$while_end1

$while_body1:
  %$28 = load i32, i32* %j_2
  %$29 = load i32*, i32** %a_1
  %$ptr2 = getelementptr inbounds i32, i32* %$29, i32 %$28
  %$30 = load i32, i32* %$ptr2
  %$31 = load i32, i32* %j_2
  %$32 = add i32 %$31, 1
  %$33 = load i32*, i32** %a_1
  %$ptr3 = getelementptr inbounds i32, i32* %$33, i32 %$32
  store i32 %$30, i32* %$ptr3
  %$34 = load i32, i32* %j_2
  %$35 = sub i32 %$34, 1
  store i32 %$35, i32* %j_2
  br label %$while_entry1

$while_end1:
  %$36 = load i32, i32* %temp_2
  %$37 = load i32, i32* %j_2
  %$38 = add i32 %$37, 1
  %$39 = load i32*, i32** %a_1
  %$ptr4 = getelementptr inbounds i32, i32* %$33, i32 %$38
  store i32 %$36, i32* %$ptr4
  %$40 = load i32, i32* %i_1
  %$41 = add i32 %$40, 1
  store i32 %$41, i32* %i_1
  br label %$while_entry0
}

define i32 @main() {
$entry:
  %tmp_5 = alloca i32
  store i32 10, i32* @n
  %a_4 = alloca [10 x i32]
  %$ptr0 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 0
  store i32 4, i32* %$ptr0
  %$ptr1 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 1
  store i32 3, i32* %$ptr1
  %$ptr2 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 2
  store i32 9, i32* %$ptr2
  %$ptr3 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 3
  store i32 2, i32* %$ptr3
  %$ptr4 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 4
  store i32 0, i32* %$ptr4
  %$ptr5 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 5
  store i32 1, i32* %$ptr5
  %$ptr6 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 6
  store i32 6, i32* %$ptr6
  %$ptr7 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 7
  store i32 5, i32* %$ptr7
  %$ptr8 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 8
  store i32 7, i32* %$ptr8
  %$ptr9 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 9
  store i32 8, i32* %$ptr9
  %i_4 = alloca i32
  %$ptr10 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 0
  %$42 = call i32 @insertsort(i32* %$ptr10)
  store i32 %$42, i32* %i_4
  br label %$while_entry2

$while_entry2:
  %$43 = load i32, i32* %i_4
  %$44 = load i32, i32* @n
  %$45 = icmp slt i32 %$43, %$44
  %$46 = zext i1 %$45 to i32
  %$47 = icmp ne i32 %$46, 0
  br i1 %$47, label %$while_body2, label %$while_end2

$while_body2:
  %$48 = load i32, i32* %i_4
  %$ptr11 = getelementptr inbounds [10 x i32], [10 x i32]* %a_4, i32 0, i32 %$48
  %$49 = load i32, i32* %$ptr11
  store i32 %$49, i32* %tmp_5
  %$50 = load i32, i32* %tmp_5
  call void @putint(i32 %$50)
  store i32 10, i32* %tmp_5
  %$51 = load i32, i32* %tmp_5
  call void @putch(i32 %$51)
  %$52 = load i32, i32* %i_4
  %$53 = add i32 %$52, 1
  store i32 %$53, i32* %i_4
  br label %$while_entry2

$while_end2:
  ret i32 0
}
